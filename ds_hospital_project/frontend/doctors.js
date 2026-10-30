
const doctorForm = document.getElementById("doctorForm");
const doctorTableBody = document.getElementById("doctorTableBody");
const doctorSearch = document.getElementById("doctorSearch");
const doctorMessage = document.getElementById("doctorMessage");

const doctorId = document.getElementById("doctorId");
const doctorName = document.getElementById("doctorName");
const doctorSpecialization = document.getElementById("doctorSpecialization");
const doctorExperience = document.getElementById("doctorExperience");
const doctorContact = document.getElementById("doctorContact");
const doctorEmail = document.getElementById("doctorEmail");

const editDoctorId = document.getElementById("editDoctorId");
const submitDoctor = document.getElementById("submitDoctor");
const cancelDoctorEdit = document.getElementById("cancelDoctorEdit");

let doctors = loadDoctors();

function loadDoctors() {
    try {
        const records = JSON.parse(localStorage.getItem("doctors") || "[]");
        return Array.isArray(records) ? records : [];
    } catch {
        return [];
    }
}

function saveDoctors() {
    try {
        localStorage.setItem("doctors", JSON.stringify(doctors));
        return true;
    } catch {
        showDoctorMessage("Unable to save records in browser storage.", true);
        return false;
    }
}

function escapeDoctorHTML(value) {
    return String(value ?? "").replace(/[&<>"']/g, character => ({
        "&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&#39;"
    })[character]);
}

function showDoctorMessage(message, isError = false) {
    doctorMessage.textContent = message;
    doctorMessage.className = isError
        ? "form-message error-message"
        : "form-message success-message";
}

function updateDoctorCounts() {
    document.getElementById("doctorCount").textContent = doctors.length;

    const specializations = new Set(
        doctors.map(doctor => doctor.specialization)
    );

    document.getElementById("specializationCount").textContent =
        specializations.size;
}

function displayDoctors(query = "") {
    const search = query.trim().toLowerCase();

    const filteredDoctors = doctors.filter(doctor =>
        String(doctor.id).toLowerCase().includes(search) ||
        doctor.name.toLowerCase().includes(search) ||
        doctor.specialization.toLowerCase().includes(search)
    );

    if (filteredDoctors.length === 0) {
        doctorTableBody.innerHTML = `
            <tr>
                <td colspan="7" class="empty-state">
                    ${doctors.length === 0
                ? "No doctors registered yet."
                : "No matching doctors found."}
                </td>
            </tr>
        `;

        updateDoctorCounts();
        return;
    }

    doctorTableBody.innerHTML = filteredDoctors.map(doctor => `
        <tr>
            <td><strong>${escapeDoctorHTML(doctor.id)}</strong></td>
            <td>${escapeDoctorHTML(doctor.name)}</td>
            <td>${escapeDoctorHTML(doctor.specialization)}</td>
            <td>${escapeDoctorHTML(doctor.experience)} years</td>
            <td>${escapeDoctorHTML(doctor.contact)}</td>
            <td>${escapeDoctorHTML(doctor.email)}</td>
            <td>
                <div class="table-actions">
                    <button type="button"
                        class="edit-button"
                        data-action="edit"
                        data-id="${escapeDoctorHTML(doctor.id)}">
                        Edit
                    </button>

                    <button type="button"
                        class="delete-button"
                        data-action="delete"
                        data-id="${escapeDoctorHTML(doctor.id)}">
                        Delete
                    </button>
                </div>
            </td>
        </tr>
    `).join("");

    updateDoctorCounts();
}

doctorForm.addEventListener("submit", event => {
    event.preventDefault();

    const id = doctorId.value.trim();
    const name = doctorName.value.trim();
    const specialization = doctorSpecialization.value;
    const experience = Number(doctorExperience.value);
    const contact = doctorContact.value.trim();
    const email = doctorEmail.value.trim();

    if (!id || !name || !specialization || !email ||
        doctorExperience.value === "") {
        showDoctorMessage("Please complete all required fields.", true);
        return;
    }

    if (!Number.isInteger(experience) ||
        experience < 0 || experience > 70) {
        showDoctorMessage("Enter experience between 0 and 70 years.", true);
        return;
    }

    if (!/^[0-9]{10}$/.test(contact)) {
        showDoctorMessage("Enter a valid 10-digit contact number.", true);
        return;
    }

    const editingId = editDoctorId.value;

    const duplicate = doctors.some(doctor =>
        doctor.id.toLowerCase() === id.toLowerCase() &&
        doctor.id !== editingId
    );

    if (duplicate) {
        showDoctorMessage("This doctor ID already exists.", true);
        return;
    }

    const updatedDoctor = {
        id,
        name,
        specialization,
        experience,
        contact,
        email
    };

    const previousDoctors = [...doctors];

    if (editingId) {
        const index = doctors.findIndex(
            doctor => doctor.id === editingId
        );

        if (index === -1) {
            showDoctorMessage("Doctor record not found.", true);
            return;
        }

        doctors[index] = updatedDoctor;
    } else {
        doctors.push(updatedDoctor);
    }

    if (!saveDoctors()) {
        doctors = previousDoctors;
        return;
    }

    const wasEditing = Boolean(editingId);

    resetDoctorForm();
    displayDoctors(doctorSearch.value);

    showDoctorMessage(
        wasEditing
            ? "Doctor record updated successfully."
            : "Doctor registered successfully."
    );
});

doctorTableBody.addEventListener("click", event => {
    const button = event.target.closest("button[data-action]");

    if (!button) return;

    const id = button.dataset.id;
    const action = button.dataset.action;

    const doctor = doctors.find(record => record.id === id);

    if (!doctor) {
        showDoctorMessage("Doctor record not found.", true);
        return;
    }

    if (action === "edit") {
        editDoctorId.value = doctor.id;
        doctorId.value = doctor.id;
        doctorId.readOnly = true;

        doctorName.value = doctor.name;
        doctorSpecialization.value = doctor.specialization;
        doctorExperience.value = doctor.experience;
        doctorContact.value = doctor.contact;
        doctorEmail.value = doctor.email;

        document.getElementById("doctorFormTitle").textContent =
            "Edit Doctor";

        submitDoctor.textContent = "Save Changes";
        cancelDoctorEdit.hidden = false;

        window.scrollTo({ top: 0, behavior: "smooth" });
        doctorName.focus();
    }

    if (action === "delete") {
        if (!confirm(`Delete doctor ${doctor.name} (${doctor.id})?`)) {
            return;
        }

        const previousDoctors = [...doctors];

        doctors = doctors.filter(record => record.id !== id);

        if (!saveDoctors()) {
            doctors = previousDoctors;
            return;
        }

        if (editDoctorId.value === id) {
            resetDoctorForm();
        }

        displayDoctors(doctorSearch.value);
        showDoctorMessage("Doctor deleted successfully.");
    }
});

doctorSearch.addEventListener("input", () => {
    displayDoctors(doctorSearch.value);
});

cancelDoctorEdit.addEventListener("click", resetDoctorForm);

function resetDoctorForm() {
    doctorForm.reset();
    editDoctorId.value = "";
    doctorId.readOnly = false;

    document.getElementById("doctorFormTitle").textContent =
        "Register New Doctor";

    submitDoctor.textContent = "+ Register Doctor";
    cancelDoctorEdit.hidden = true;
}

displayDoctors();

