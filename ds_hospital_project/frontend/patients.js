
const API_URL = "http://127.0.0.1:8080/api/patients";

const patientForm = document.getElementById("patientForm");
const patientTableBody = document.getElementById("patientTableBody");
const patientSearch = document.getElementById("patientSearch");
const patientMessage = document.getElementById("patientMessage");

const patientId = document.getElementById("patientId");
const patientName = document.getElementById("patientName");
const patientAge = document.getElementById("patientAge");
const patientGender = document.getElementById("patientGender");
const patientContact = document.getElementById("patientContact");
const patientDisease = document.getElementById("patientDisease");

const editPatientId = document.getElementById("editPatientId");
const submitPatient = document.getElementById("submitPatient");
const cancelEdit = document.getElementById("cancelEdit");

let patients = [];

function escapeHTML(value) {
    return String(value ?? "").replace(/[&<>"']/g, character => ({
        "&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&#39;"
    })[character]);
}

function showMessage(message, isError = false) {
    patientMessage.textContent = message;
    patientMessage.className = isError
        ? "form-message error-message"
        : "form-message success-message";
}

function updatePatientCounts() {
    document.getElementById("patientCount").textContent = patients.length;
    document.getElementById("recordCount").textContent = patients.length;
}

async function loadPatients() {
    try {
        const response = await fetch(API_URL);

        if (!response.ok) {
            throw new Error("Unable to retrieve patient records.");
        }

        const data = await response.json();

        patients = (data.patients || []).map(patient => ({
            ...patient,
            id: String(patient.id),
            contact: patient.contact || ""
        }));

        displayPatients(patientSearch.value);
    } catch (error) {
        showMessage(
            "Cannot connect to the C backend. Make sure server.exe is running.",
            true
        );

        patients = [];
        displayPatients(patientSearch.value);
    }
}

function displayPatients(query = "") {
    const search = query.trim().toLowerCase();

    const filteredPatients = patients.filter(patient =>
        String(patient.id).toLowerCase().includes(search) ||
        String(patient.name).toLowerCase().includes(search)
    );

    if (filteredPatients.length === 0) {
        patientTableBody.innerHTML = `
            <tr>
                <td colspan="7" class="empty-state">
                    ${patients.length === 0
                ? "No patients registered yet."
                : "No matching patients found."}
                </td>
            </tr>
        `;

        updatePatientCounts();
        return;
    }

    patientTableBody.innerHTML = filteredPatients.map(patient => `
        <tr>
            <td><strong>${escapeHTML(patient.id)}</strong></td>
            <td>${escapeHTML(patient.name)}</td>
            <td>${escapeHTML(patient.age)}</td>
            <td>${escapeHTML(patient.gender)}</td>
            <td>${escapeHTML(patient.contact)}</td>
            <td>${escapeHTML(patient.disease)}</td>
            <td>
                <div class="table-actions">
                    <button
                        type="button"
                        class="edit-button"
                        data-action="edit"
                        data-id="${escapeHTML(patient.id)}">
                        Edit
                    </button>
                    <button
                        type="button"
                        class="delete-button"
                        data-action="delete"
                        data-id="${escapeHTML(patient.id)}">
                        Delete
                    </button>
                </div>
            </td>
        </tr>
    `).join("");

    updatePatientCounts();
}

patientForm.addEventListener("submit", async event => {
    event.preventDefault();

    const id = patientId.value.trim();
    const name = patientName.value.trim();
    const age = Number(patientAge.value);
    const gender = patientGender.value;
    const contact = patientContact.value.trim();
    const disease = patientDisease.value.trim();

    if (!id || !name || !gender || !disease || !patientAge.value) {
        showMessage("Please complete all required fields.", true);
        return;
    }

    if (!Number.isInteger(age) || age < 1 || age > 120) {
        showMessage("Enter a valid age between 1 and 120.", true);
        return;
    }

    if (!/^[0-9]{10}$/.test(contact)) {
        showMessage("Enter a valid 10-digit contact number.", true);
        return;
    }

    const editingId = editPatientId.value;

    if (editingId) {
        const updatedPatient = {
            id: Number(editingId),
            name,
            age,
            gender,
            disease
        };

        submitPatient.disabled = true;

        try {
            const response = await fetch(API_URL, {
                method: "PUT",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify(updatedPatient)
            });

            const result = await response.json();

            if (!response.ok) {
                throw new Error(result.error || "Failed to update patient.");
            }

            resetPatientForm();
            await loadPatients();

            showMessage("Patient updated successfully.");
        } catch (error) {
            showMessage(error.message, true);
        } finally {
            submitPatient.disabled = false;
        }

        return;
    }

    if (patients.some(patient => patient.id === id)) {
        showMessage("This patient ID already exists.", true);
        return;
    }

    const newPatient = {
        id: Number(id),
        name,
        age,
        gender,
        disease
    };

    submitPatient.disabled = true;

    try {
        const response = await fetch(API_URL, {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(newPatient)
        });

        const result = await response.json();

        if (!response.ok) {
            throw new Error(result.error || "Patient registration failed.");
        }

        resetPatientForm();
        await loadPatients();
        showMessage("Patient registered successfully in the C backend.");
    } catch (error) {
        showMessage(error.message, true);
    } finally {
        submitPatient.disabled = false;
    }
});

patientTableBody.addEventListener("click", async event => {
    const button = event.target.closest("button[data-action]");

    if (!button) return;

    const patient = patients.find(
        record => record.id === button.dataset.id
    );

    if (!patient) {
        showMessage("Patient record not found.", true);
        return;
    }

    if (button.dataset.action === "edit") {
        editPatientId.value = patient.id;
        patientId.value = patient.id;
        patientId.readOnly = true;

        patientName.value = patient.name;
        patientAge.value = patient.age;
        patientGender.value = patient.gender;
        patientContact.value = patient.contact || "";
        patientDisease.value = patient.disease;

        document.getElementById("formTitle").textContent = "Edit Patient";
        submitPatient.textContent = "Save Changes";
        cancelEdit.hidden = false;

        window.scrollTo({
            top: 0,
            behavior: "smooth"
        });

        patientName.focus();
    }


    if (button.dataset.action === "delete") {
        const confirmed = confirm(
            `Are you sure you want to delete patient ${patient.name} (ID: ${patient.id})?`
        );

        if (!confirmed) return;

        button.disabled = true;

        try {
            const response = await fetch(API_URL, {
                method: "DELETE",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify({
                    id: Number(patient.id)
                })
            });

            const result = await response.json();

            if (!response.ok) {
                throw new Error(
                    result.error || "Failed to delete patient."
                );
            }

            await loadPatients();
            showMessage("Patient deleted successfully.");

        } catch (error) {
            showMessage(
                error.message || "Unable to delete patient.",
                true
            );
            button.disabled = false;
        }
    }


});

patientSearch.addEventListener("input", () => {
    displayPatients(patientSearch.value);
});

cancelEdit.addEventListener("click", resetPatientForm);

function resetPatientForm() {
    patientForm.reset();
    editPatientId.value = "";
    patientId.readOnly = false;

    document.getElementById("formTitle").textContent =
        "Register New Patient";

    submitPatient.textContent = "+ Register Patient";
    cancelEdit.hidden = true;
}

loadPatients();

