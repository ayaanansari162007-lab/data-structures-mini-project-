
const API_URL = "http://127.0.0.1:8080/api";

const form = document.getElementById("appointmentForm");
const tableBody = document.getElementById("appointmentTableBody");
const message = document.getElementById("appointmentMessage");

const patientSelect = document.getElementById("appointmentPatient");
const doctorSelect = document.getElementById("appointmentDoctor");
const appointmentIdInput = document.getElementById("appointmentId");
const dateInput = document.getElementById("appointmentDate");
const timeInput = document.getElementById("appointmentTime");

const submitButton = form.querySelector('button[type="submit"]');

let appointments = [];
let patients = [];
let doctors = [];

function escapeHTML(value) {
    return String(value ?? "").replace(/[&<>"']/g, character => ({
        "&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&#39;"
    })[character]);
}

function showMessage(text, isError = false) {
    message.textContent = text;
    message.className = isError
        ? "appointment-error"
        : "appointment-success";
}

async function requestJSON(url, options = {}) {
    const response = await fetch(url, options);
    const text = await response.text();

    let result = {};

    try {
        result = text ? JSON.parse(text) : {};
    } catch {
        throw new Error(
            "Invalid server response. Check the C backend."
        );
    }

    if (!response.ok) {
        throw new Error(
            result.error || `Request failed: HTTP ${response.status}`
        );
    }

    return result;
}

async function loadPatients() {
    try {
        const data = await requestJSON(`${API_URL}/patients`);

        patients = data.patients || [];

        patientSelect.innerHTML =
            '<option value="">Select patient</option>';

        patients.forEach(patient => {
            const option = document.createElement("option");

            option.value = patient.id;
            option.textContent =
                `${patient.name} (ID: ${patient.id})`;

            patientSelect.appendChild(option);
        });

    } catch (error) {
        showMessage(
            "Could not load patients: " + error.message,
            true
        );
    }
}

function loadDoctors() {
    try {
        doctors = JSON.parse(
            localStorage.getItem("doctors") || "[]"
        );

        doctorSelect.innerHTML =
            '<option value="">Select doctor</option>';

        doctors.forEach(doctor => {
            const option = document.createElement("option");

            option.value = doctor.name;
            option.textContent =
                `${doctor.name} - ${doctor.specialization || "General"}`;

            doctorSelect.appendChild(option);
        });

    } catch (error) {
        doctors = [];
        showMessage("Could not load doctors.", true);
    }
}

async function loadAppointments() {
    try {
        const data = await requestJSON(
            `${API_URL}/appointments`
        );

        appointments = data.appointments || [];

        renderAppointments();
        updateCounts();

    } catch (error) {
        tableBody.innerHTML = `
            <tr>
                <td colspan="7">
                    Could not load appointments.
                </td>
            </tr>
        `;

        showMessage(
            "Appointment API error: " + error.message,
            true
        );
    }
}

function renderAppointments() {
    if (appointments.length === 0) {
        tableBody.innerHTML = `
            <tr>
                <td colspan="7">No appointments booked yet.</td>
            </tr>
        `;
        return;
    }

    tableBody.innerHTML = appointments.map(appointment => `
        <tr>
            <td>${escapeHTML(appointment.appointmentId)}</td>
            <td>${escapeHTML(appointment.patientName)}</td>
            <td>${escapeHTML(appointment.doctorName)}</td>
            <td>${escapeHTML(appointment.date)}</td>
            <td>${escapeHTML(appointment.time)}</td>
            <td>Waiting</td>
            <td>
                <button
                    class="appointment-delete"
                    type="button"
                    data-delete="${escapeHTML(appointment.appointmentId)}">
                    Cancel
                </button>
            </td>
        </tr>
    `).join("");
}

function updateCounts() {
    const appointmentCount =
        document.getElementById("appointmentCount");

    const waitingCount =
        document.getElementById("waitingCount");

    if (appointmentCount) {
        appointmentCount.textContent = appointments.length;
    }

    if (waitingCount) {
        waitingCount.textContent = appointments.length;
    }
}

form.addEventListener("submit", async event => {
    event.preventDefault();

    const patientId = Number(patientSelect.value);

    const patient = patients.find(
        item => Number(item.id) === patientId
    );

    const appointment = {
        appointmentId: Number(appointmentIdInput.value),
        patientId,
        patientName: patient ? patient.name : "",
        doctorName: doctorSelect.value,
        date: dateInput.value,
        time: timeInput.value
    };

    if (
        !Number.isInteger(appointment.appointmentId) ||
        appointment.appointmentId <= 0 ||
        !patient ||
        !appointment.doctorName ||
        !appointment.date ||
        !appointment.time
    ) {
        showMessage(
            "Please complete every field with valid information.",
            true
        );
        return;
    }

    submitButton.disabled = true;

    try {
        await requestJSON(`${API_URL}/appointments`, {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(appointment)
        });

        form.reset();

        await loadAppointments();

        showMessage("Appointment added successfully.");

    } catch (error) {
        showMessage(error.message, true);

    } finally {
        submitButton.disabled = false;
    }
});

tableBody.addEventListener("click", async event => {
    const button = event.target.closest(
        "button[data-delete]"
    );

    if (!button) {
        return;
    }

    const appointmentId = Number(button.dataset.delete);

    if (!confirm(`Cancel appointment ${appointmentId}?`)) {
        return;
    }

    button.disabled = true;

    try {
        await requestJSON(`${API_URL}/appointments`, {
            method: "DELETE",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({ appointmentId })
        });

        await loadAppointments();

        showMessage("Appointment cancelled successfully.");

    } catch (error) {
        showMessage(error.message, true);
        button.disabled = false;
    }
});

async function initializeAppointments() {
    loadDoctors();

    await loadPatients();

    await loadAppointments();
}

initializeAppointments();

