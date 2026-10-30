
const API_URL = "http://127.0.0.1:8080/api";

const emergencyForm = document.getElementById("emergencyForm");
const emergencyTableBody = document.getElementById("emergencyTableBody");
const emergencyMessage = document.getElementById("emergencyMessage");

const emergencyId = document.getElementById("emergencyId");
const emergencyPatient = document.getElementById("emergencyPatient");
const emergencyCondition = document.getElementById("emergencyCondition");
const emergencyPriority = document.getElementById("emergencyPriority");

let emergencies = loadEmergencies();
let patients = [];

function loadEmergencies() {
    try {
        const records = JSON.parse(
            localStorage.getItem("emergencies") || "[]"
        );

        return Array.isArray(records) ? records : [];
    } catch {
        return [];
    }
}

async function fetchPatients() {
    try {
        const response = await fetch(`${API_URL}/patients`);
        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || "Could not load patients.");
        }

        patients = data.patients || [];
        populateEmergencyPatients();

        return true;
    } catch (error) {
        patients = [];

        emergencyPatient.innerHTML =
            '<option value="">Could not load patients</option>';

        showEmergencyMessage(
            "Could not load patients. Make sure server.exe is running.",
            true
        );

        console.error("Patient loading error:", error);

        return false;
    }
}

function saveEmergencies() {
    try {
        localStorage.setItem(
            "emergencies",
            JSON.stringify(emergencies)
        );

        return true;
    } catch {
        showEmergencyMessage(
            "Unable to save emergency records.",
            true
        );

        return false;
    }
}

function escapeEmergencyHTML(value) {
    return String(value ?? "").replace(/[&<>"']/g, character => ({
        "&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&#39;"
    })[character]);
}

function showEmergencyMessage(text, isError = false) {
    emergencyMessage.textContent = text;

    emergencyMessage.className = isError
        ? "form-message error-message"
        : "form-message success-message";
}

function populateEmergencyPatients() {
    emergencyPatient.innerHTML =
        '<option value="">Select patient</option>';

    patients.forEach(patient => {
        const option = document.createElement("option");

        option.value = String(patient.id);

        option.textContent =
            `${patient.id} - ${patient.name}`;

        emergencyPatient.appendChild(option);
    });

    if (patients.length === 0) {
        emergencyPatient.innerHTML =
            '<option value="">No registered patients found</option>';
    }
}

function priorityName(priority) {
    return {
        1: "Critical",
        2: "High",
        3: "Medium",
        4: "Low"
    }[priority] || "Unknown";
}

function updateEmergencyCounts() {
    const emergencyCount =
        document.getElementById("emergencyCount");

    const activeEmergencyCount =
        document.getElementById("activeEmergencyCount");

    if (emergencyCount) {
        emergencyCount.textContent = emergencies.length;
    }

    if (activeEmergencyCount) {
        activeEmergencyCount.textContent =
            emergencies.filter(
                item => item.status === "Waiting"
            ).length;
    }
}

function displayEmergencies() {
    const sortedEmergencies = [...emergencies].sort(
        (a, b) =>
            Number(a.priority) - Number(b.priority) ||
            Number(a.createdAt) - Number(b.createdAt)
    );

    if (sortedEmergencies.length === 0) {
        emergencyTableBody.innerHTML = `
            <tr>
                <td colspan="6" class="empty-state">
                    No emergency cases registered yet.
                </td>
            </tr>
        `;

        updateEmergencyCounts();
        return;
    }

    emergencyTableBody.innerHTML =
        sortedEmergencies.map(item => `
            <tr>
                <td>
                    <strong>${escapeEmergencyHTML(item.id)}</strong>
                </td>

                <td>
                    ${escapeEmergencyHTML(item.patientName)}
                </td>

                <td>
                    ${escapeEmergencyHTML(item.condition)}
                </td>

                <td>
                    <span class="priority-badge priority-${Number(item.priority)}">
                        ${priorityName(item.priority)}
                    </span>
                </td>

                <td>
                    ${escapeEmergencyHTML(item.status)}
                </td>

                <td>
                    ${item.status === "Waiting"
                ? `<button
                                type="button"
                                class="edit-button"
                                data-action="complete"
                                data-id="${escapeEmergencyHTML(item.id)}">
                                Mark Treated
                            </button>`
                : "—"
            }
                </td>
            </tr>
        `).join("");

    updateEmergencyCounts();
}

emergencyForm.addEventListener("submit", async event => {
    event.preventDefault();

    const id = emergencyId.value.trim();
    const patientId = String(emergencyPatient.value);
    const condition = emergencyCondition.value.trim();
    const priority = Number(emergencyPriority.value);

    if (!id || !patientId || !condition || !priority) {
        showEmergencyMessage(
            "Please complete every field.",
            true
        );
        return;
    }

    const patient = patients.find(
        record => String(record.id) === patientId
    );

    if (!patient) {
        showEmergencyMessage(
            "Please select a registered patient.",
            true
        );
        return;
    }

    if (
        emergencies.some(
            item => String(item.id).toLowerCase() === id.toLowerCase()
        )
    ) {
        showEmergencyMessage(
            "Emergency case ID already exists.",
            true
        );
        return;
    }

    const newEmergency = {
        id,
        patientId,
        patientName: patient.name,
        condition,
        priority,
        status: "Waiting",
        createdAt: Date.now()
    };

    emergencies.push(newEmergency);

    if (!saveEmergencies()) {
        emergencies.pop();
        return;
    }

    emergencyForm.reset();

    displayEmergencies();

    showEmergencyMessage(
        "Emergency case registered successfully."
    );
});

emergencyTableBody.addEventListener("click", event => {
    const button = event.target.closest(
        "button[data-action='complete']"
    );

    if (!button) {
        return;
    }

    const item = emergencies.find(
        record => String(record.id) === button.dataset.id
    );

    if (!item || item.status !== "Waiting") {
        return;
    }

    if (!confirm(`Mark emergency case ${item.id} as treated?`)) {
        return;
    }

    const previousStatus = item.status;

    item.status = "Treated";

    if (!saveEmergencies()) {
        item.status = previousStatus;
        return;
    }

    displayEmergencies();

    showEmergencyMessage(
        "Emergency case marked as treated."
    );
});

async function initializeEmergencyPage() {
    displayEmergencies();
    await fetchPatients();
}

initializeEmergencyPage();
