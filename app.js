/* =====================================================================
   Scholaris frontend logic.
   - Handles authentication (SHA-256 in browser via Web Crypto API)
   - Talks to the C++ backend HTTP server (http://localhost:8080)
   - Falls back to localStorage persistence per user.
   - Plays a Web Audio beep for reminder sounds (no external file needed).
   ===================================================================== */

const API = "http://localhost:8080";  // C++ backend endpoint

/* ---------- STATE ---------- */
let currentUser = null;          // logged-in user id
let tasks      = [];
let expenses   = [];
let habits     = [];
let healthLog  = [];
let reminders  = [];

/* ---------- HASHING (SHA-256 via Web Crypto) ---------- */
/* SHA-256 is industry standard. Node's bcrypt is heavier;
   for a browser-side demo this is the strongest available primitive. */
async function sha256(text) {
  const buf = new TextEncoder().encode(text);
  const hash = await crypto.subtle.digest("SHA-256", buf);
  return Array.from(new Uint8Array(hash))
    .map(b => b.toString(16).padStart(2, "0"))
    .join("");
}

/* ---------- AUTH ---------- */
const usersKey = "scholaris_users";

function getUsers() {
  return JSON.parse(localStorage.getItem(usersKey) || "{}");
}
function saveUsers(u) {
  localStorage.setItem(usersKey, JSON.stringify(u));
}

document.querySelectorAll(".tab-btn").forEach(b => {
  b.onclick = () => {
    document.querySelectorAll(".tab-btn").forEach(x => x.classList.remove("active"));
    document.querySelectorAll(".auth-form").forEach(x => x.classList.remove("active"));
    b.classList.add("active");
    document.getElementById(b.dataset.tab + "-form").classList.add("active");
  };
});

document.getElementById("signup-form").onsubmit = async e => {
  e.preventDefault();
  const u = document.getElementById("signup-username").value.trim();
  const p = document.getElementById("signup-password").value;
  const msg = document.getElementById("signup-msg");
  const users = getUsers();
  if (users[u]) {
    msg.className = "auth-msg error";
    msg.textContent = "User already exists.";
    return;
  }
  users[u] = { hash: await sha256(p), created: Date.now() };
  saveUsers(users);
  msg.className = "auth-msg success";
  msg.textContent = "Account created. You can now sign in.";
};

document.getElementById("login-form").onsubmit = async e => {
  e.preventDefault();
  const u = document.getElementById("login-username").value.trim();
  const p = document.getElementById("login-password").value;
  const msg = document.getElementById("login-msg");
  const users = getUsers();
  if (!users[u] || users[u].hash !== await sha256(p)) {
    msg.className = "auth-msg error";
    msg.textContent = "Invalid credentials.";
    return;
  }
  loginSuccess(u);
};

function loginSuccess(username) {
  currentUser = username;
  document.getElementById("auth-screen").style.display = "none";
  document.getElementById("app").classList.remove("app-hidden");
  document.getElementById("current-user").textContent = username;
  loadUserData();
  initRoadmapDropdowns();
  refreshAll();
}

document.getElementById("logout-btn").onclick = () => {
  saveUserData();
  currentUser = null;
  location.reload();
};

/* ---------- DATA PERSISTENCE (per user) ---------- */
function userKey() { return "scholaris_data_" + currentUser; }

function saveUserData() {
  if (!currentUser) return;
  localStorage.setItem(userKey(), JSON.stringify({
    tasks, expenses, habits, healthLog, reminders
  }));
}
function loadUserData() {
  const raw = localStorage.getItem(userKey());
  if (raw) {
    const d = JSON.parse(raw);
    tasks      = d.tasks      || [];
    expenses   = d.expenses   || [];
    habits     = d.habits     || [];
    healthLog  = d.healthLog  || [];
    reminders  = d.reminders  || [];
  }
}

/* ---------- BACKEND COMMUNICATION ---------- */
/* The C++ backend exposes endpoints like /sort_tasks, /knapsack, etc.
   If unreachable, we fall back to JS implementations of the same
   algorithms so the app still works offline. */

async function callBackend(endpoint, payload) {
  try {
    const r = await fetch(API + endpoint, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });
    if (!r.ok) throw new Error("backend error");
    return await r.json();
  } catch (e) {
    return null;  // signal fallback
  }
}

/* ---------- NAVIGATION ---------- */
document.querySelectorAll(".nav-btn").forEach(b => {
  b.onclick = () => {
    document.querySelectorAll(".nav-btn").forEach(x => x.classList.remove("active"));
    document.querySelectorAll(".panel").forEach(x => x.classList.remove("active"));
    b.classList.add("active");
    document.getElementById("section-" + b.dataset.section).classList.add("active");
  };
});

/* ============================================================
   STUDY PLANNER
   ============================================================ */
let nextTaskId = 1;
function addTask() {
  const subject = document.getElementById("task-subject").value.trim();
  const deadline = +document.getElementById("task-deadline").value;
  const duration = +document.getElementById("task-duration").value;
  const priority = +document.getElementById("task-priority").value;
  if (!subject || !deadline || !duration || !priority) return;
  tasks.push({ id: nextTaskId++, subject, deadline, duration, priority });
  ["task-subject","task-deadline","task-duration","task-priority"]
    .forEach(id => document.getElementById(id).value = "");
  saveUserData();
  renderTasks();
}

function renderTasks() {
  const el = document.getElementById("task-list");
  if (!tasks.length) { el.innerHTML = '<div class="empty-state">No tasks yet.</div>'; return; }
  el.innerHTML = tasks.map(t => `
    <div class="item">
      <div class="item-main">
        <div class="item-title">${t.subject}</div>
        <div class="item-sub">ID ${t.id} · ${t.duration}h needed · due in ${t.deadline} day(s)</div>
      </div>
      <div class="item-meta">
        <span class="tag priority">Priority ${t.priority}</span>
      </div>
    </div>`).join("");
}

async function sortTasks(by) {
  const res = await callBackend("/sort_tasks", { tasks, by });
  if (res) tasks = res.tasks;
  else {
    // Fallback: same logic as backend (quick sort)
    tasks.sort((a,b) => by === "deadline" ? a.deadline - b.deadline : b.priority - a.priority);
  }
  saveUserData();
  renderTasks();
  document.getElementById("algo-result").textContent =
    `Tasks sorted by ${by} (${res ? "C++ backend" : "JS fallback"}).`;
}

async function runGreedy() {
  /* Greedy: pick tasks ordered by deadline that fit in the daily-hour budget. */
  const res = await callBackend("/greedy", { tasks });
  let scheduled;
  if (res) scheduled = res.scheduled;
  else {
    const sorted = [...tasks].sort((a,b)=>a.deadline-b.deadline);
    let t = 0; scheduled = [];
    for (const x of sorted) {
      if (t + x.duration <= x.deadline * 8) { scheduled.push(x); t += x.duration; }
    }
  }
  document.getElementById("algo-result").textContent =
    "Greedy schedule:\n" + scheduled.map(t=>`• ${t.subject} (${t.duration}h)`).join("\n");
}

async function runKnapsack() {
  /* 0/1 knapsack: maximize total priority within available time budget. */
  const time = +document.getElementById("knapsack-time").value;
  if (!time) return;
  const res = await callBackend("/knapsack", { tasks, time });
  let selected, totalPriority;
  if (res) { selected = res.selected; totalPriority = res.priority; }
  else {
    const n = tasks.length;
    const dp = Array.from({length:n+1}, () => Array(time+1).fill(0));
    for (let i = 1; i <= n; i++) for (let w = 0; w <= time; w++) {
      dp[i][w] = tasks[i-1].duration <= w
        ? Math.max(dp[i-1][w], dp[i-1][w-tasks[i-1].duration] + tasks[i-1].priority)
        : dp[i-1][w];
    }
    selected = []; let w = time;
    for (let i = n; i > 0 && w > 0; i--) {
      if (dp[i][w] !== dp[i-1][w]) { selected.push(tasks[i-1]); w -= tasks[i-1].duration; }
    }
    totalPriority = dp[n][time];
  }
  document.getElementById("algo-result").textContent =
    `Knapsack (max priority = ${totalPriority}):\n` +
    selected.map(t=>`• ${t.subject} (${t.duration}h, priority ${t.priority})`).join("\n");
}

/* ============================================================
   EXPENSES
   ============================================================ */
let nextExpenseId = 1;
function addExpense() {
  const amount = +document.getElementById("exp-amount").value;
  const category = document.getElementById("exp-category").value.trim();
  if (!amount || !category) return;
  expenses.push({ id: nextExpenseId++, amount, category });
  document.getElementById("exp-amount").value = "";
  document.getElementById("exp-category").value = "";
  saveUserData();
  renderExpenses();
}
function renderExpenses() {
  const el = document.getElementById("expense-list");
  if (!expenses.length) { el.innerHTML = '<div class="empty-state">No expenses yet.</div>'; return; }
  let total = 0;
  el.innerHTML = expenses.map(e => {
    total += e.amount;
    return `<div class="item">
      <div class="item-main">
        <div class="item-title">${e.category}</div>
        <div class="item-sub">ID ${e.id}</div>
      </div>
      <div class="item-meta"><span class="tag">Rs. ${e.amount.toFixed(2)}</span></div>
    </div>`;
  }).join("");
  document.getElementById("expense-total").textContent = `Total: Rs. ${total.toFixed(2)}`;
}
async function sortExpenses() {
  /* Merge sort. Backend has the canonical version; JS mirrors it. */
  const res = await callBackend("/sort_expenses", { expenses });
  if (res) expenses = res.expenses;
  else expenses.sort((a,b)=>a.amount - b.amount);
  saveUserData();
  renderExpenses();
}
async function searchExpense() {
  /* Binary search — requires sorted data. */
  await sortExpenses();
  const amt = +document.getElementById("search-amt").value;
  let lo = 0, hi = expenses.length - 1, found = -1;
  while (lo <= hi) {
    const mid = (lo + hi) >> 1;
    if (Math.abs(expenses[mid].amount - amt) < 0.01) { found = mid; break; }
    expenses[mid].amount < amt ? lo = mid+1 : hi = mid-1;
  }
  document.getElementById("expense-total").textContent =
    found === -1 ? "Not found." :
    `Found: ${expenses[found].category} — Rs. ${expenses[found].amount.toFixed(2)}`;
}

/* ============================================================
   HABITS
   ============================================================ */
let nextHabitId = 1;
function addHabit() {
  const name = document.getElementById("habit-name").value.trim();
  const time = +document.getElementById("habit-time").value;
  if (!name || !time) return;
  habits.push({ id: nextHabitId++, name, timeSpent: time });
  document.getElementById("habit-name").value = "";
  document.getElementById("habit-time").value = "";
  saveUserData();
  renderHabits();
}
function renderHabits() {
  const el = document.getElementById("habit-list");
  if (!habits.length) { el.innerHTML = '<div class="empty-state">No habits yet.</div>'; return; }
  el.innerHTML = habits.map(h => `
    <div class="item">
      <div class="item-main">
        <div class="item-title">${h.name}</div>
        <div class="item-sub">ID ${h.id}</div>
      </div>
      <div class="item-meta"><span class="tag">${h.timeSpent} min/day</span></div>
    </div>`).join("");
}
function totalHabitTime() {
  const total = habits.reduce((s,h)=>s+h.timeSpent, 0);
  document.getElementById("habit-total").textContent =
    `Total: ${total} minutes (${Math.floor(total/60)}h ${total%60}m)`;
}

/* ============================================================
   HEALTH
   ============================================================ */
let currentDay = 1;
function addHealth() {
  const sleepHours = +document.getElementById("health-sleep").value;
  const waterIntake = +document.getElementById("health-water").value;
  if (!sleepHours || !waterIntake) return;
  healthLog.push({ day: currentDay++, sleepHours, waterIntake });
  document.getElementById("health-sleep").value = "";
  document.getElementById("health-water").value = "";
  saveUserData();
  renderHealth();
}
function renderHealth() {
  const el = document.getElementById("health-list");
  if (!healthLog.length) { el.innerHTML = '<div class="empty-state">No health data yet.</div>'; return; }
  el.innerHTML = healthLog.map(h => `
    <div class="item">
      <div class="item-main">
        <div class="item-title">Day ${h.day}</div>
        <div class="item-sub">${h.sleepHours} hrs sleep · ${h.waterIntake} L water</div>
      </div>
    </div>`).join("");
}
function recommendStudy() {
  if (!healthLog.length) {
    document.getElementById("health-recommend").textContent = "Log at least one entry first.";
    return;
  }
  const last = healthLog[healthLog.length-1].sleepHours;
  const hrs = last >= 7 ? 8 : last >= 5 ? 6 : 4;
  document.getElementById("health-recommend").textContent =
    `Based on ${last} hrs sleep, recommended study time: ${hrs} hours.`;
}

/* ============================================================
   REMINDERS — with sound
   ============================================================ */
function generateReminders() {
  reminders = tasks
    .filter(t => t.priority >= 7 || t.deadline <= 3)
    .map(t => ({ taskId: t.id, subject: t.subject, deadline: t.deadline, priority: t.priority }))
    .sort((a,b) => b.priority - a.priority || a.deadline - b.deadline);
  saveUserData();
  renderReminders();
  if (reminders.some(r => r.deadline <= 1)) playBeep(880, 0.4);  // urgent alert
}
function renderReminders() {
  const el = document.getElementById("reminder-list");
  if (!reminders.length) { el.innerHTML = '<div class="empty-state">No reminders.</div>'; return; }
  el.innerHTML = reminders.map(r => {
    const tag = r.deadline <= 1 ? '<span class="tag urgent">Urgent</span>'
              : r.deadline <= 3 ? '<span class="tag soon">Soon</span>' : '';
    return `<div class="item">
      <div class="item-main">
        <div class="item-title">${r.subject}</div>
        <div class="item-sub">Task #${r.taskId} · due in ${r.deadline} day(s)</div>
      </div>
      <div class="item-meta">
        <span class="tag priority">Priority ${r.priority}</span>${tag}
      </div>
    </div>`;
  }).join("");
}

/* Web Audio API beep — no external file required. */
function playBeep(freq=660, duration=0.25) {
  try {
    const ctx = new (window.AudioContext || window.webkitAudioContext)();
    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.connect(gain); gain.connect(ctx.destination);
    osc.type = "sine"; osc.frequency.value = freq;
    gain.gain.setValueAtTime(0.0001, ctx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.25, ctx.currentTime + 0.02);
    gain.gain.exponentialRampToValueAtTime(0.0001, ctx.currentTime + duration);
    osc.start();
    osc.stop(ctx.currentTime + duration + 0.05);
  } catch(e) {}
}
function testSound() { playBeep(660, 0.3); setTimeout(()=>playBeep(880, 0.4), 320); }

/* ============================================================
   LEARNING ROADMAP — Dijkstra's algorithm
   Nodes: topic prerequisites in CS curriculum.
   Edges: weights = relative time/difficulty units.
   ============================================================ */
const ROADMAP = {
  nodes: [
    "Basics of Programming", "Data Types", "Control Flow",
    "Functions", "Arrays", "Pointers",
    "Recursion", "OOP", "Linked Lists",
    "Stacks & Queues", "Trees", "Graphs",
    "Sorting Algorithms", "Searching Algorithms", "Dynamic Programming",
    "Greedy Algorithms", "Graph Algorithms", "Advanced DSA"
  ],
  /* adjacency list: { from: [[to, weight], ...] } */
  edges: {
    "Basics of Programming": [["Data Types",1],["Control Flow",2]],
    "Data Types":            [["Control Flow",1],["Functions",2]],
    "Control Flow":          [["Functions",1],["Arrays",3]],
    "Functions":             [["Arrays",2],["Recursion",3]],
    "Arrays":                [["Pointers",2],["Sorting Algorithms",3],["Searching Algorithms",2]],
    "Pointers":              [["Linked Lists",2],["OOP",3]],
    "Recursion":             [["Linked Lists",2],["Trees",3],["Dynamic Programming",4]],
    "OOP":                   [["Linked Lists",1],["Advanced DSA",6]],
    "Linked Lists":          [["Stacks & Queues",2],["Trees",3]],
    "Stacks & Queues":       [["Trees",2],["Graphs",3]],
    "Trees":                 [["Graphs",2],["Advanced DSA",4]],
    "Graphs":                [["Graph Algorithms",2],["Advanced DSA",3]],
    "Sorting Algorithms":    [["Searching Algorithms",1],["Greedy Algorithms",3]],
    "Searching Algorithms":  [["Dynamic Programming",4]],
    "Dynamic Programming":   [["Advanced DSA",3]],
    "Greedy Algorithms":     [["Graph Algorithms",2],["Advanced DSA",3]],
    "Graph Algorithms":      [["Advanced DSA",2]],
    "Advanced DSA":          []
  }
};

function initRoadmapDropdowns() {
  const start = document.getElementById("rm-start");
  const end   = document.getElementById("rm-end");
  start.innerHTML = ROADMAP.nodes.map(n => `<option>${n}</option>`).join("");
  end.innerHTML   = ROADMAP.nodes.map(n => `<option>${n}</option>`).join("");
  end.value = ROADMAP.nodes[ROADMAP.nodes.length-1];
}

/* Dijkstra's algorithm — classical implementation with a simple priority queue.
   Returns shortest weighted path between two topic nodes. */
function dijkstra(graph, start, end) {
  const dist = {}, prev = {};
  graph.nodes.forEach(n => { dist[n] = Infinity; prev[n] = null; });
  dist[start] = 0;
  const visited = new Set();
  const queue = [...graph.nodes];

  while (queue.length) {
    // pick node with smallest distance
    queue.sort((a,b) => dist[a] - dist[b]);
    const u = queue.shift();
    if (dist[u] === Infinity) break;
    if (u === end) break;
    visited.add(u);
    for (const [v, w] of (graph.edges[u] || [])) {
      if (visited.has(v)) continue;
      const alt = dist[u] + w;
      if (alt < dist[v]) { dist[v] = alt; prev[v] = u; }
    }
  }
  // reconstruct path
  const path = [];
  let cur = end;
  while (cur) { path.unshift(cur); cur = prev[cur]; }
  if (path[0] !== start) return { path: [], cost: Infinity };
  return { path, cost: dist[end] };
}

async function generateRoadmap() {
  const start = document.getElementById("rm-start").value;
  const end   = document.getElementById("rm-end").value;
  let result = await callBackend("/dijkstra", { start, end });
  if (!result) result = dijkstra(ROADMAP, start, end);
  const out = document.getElementById("roadmap-result");
  const graph = document.getElementById("roadmap-graph");
  if (!result.path.length) {
    out.textContent = "No path found between selected topics.";
    graph.innerHTML = "";
    return;
  }
  out.textContent =
    `Optimal path (total weight = ${result.cost}):\n` +
    result.path.join(" -> ");
  graph.innerHTML = result.path
    .map(n => `<span class="path-node">${n}</span>`)
    .join('<span class="path-arrow">-></span>');
}

/* ---------- INITIAL RENDER ---------- */
function refreshAll() {
  // ensure id counters continue past loaded data
  if (tasks.length)    nextTaskId    = Math.max(...tasks.map(t=>t.id)) + 1;
  if (expenses.length) nextExpenseId = Math.max(...expenses.map(e=>e.id)) + 1;
  if (habits.length)   nextHabitId   = Math.max(...habits.map(h=>h.id)) + 1;
  if (healthLog.length) currentDay   = Math.max(...healthLog.map(h=>h.day)) + 1;
  renderTasks(); renderExpenses(); renderHabits(); renderHealth(); renderReminders();
}

/* Auto-save on tab close */
window.addEventListener("beforeunload", saveUserData);
