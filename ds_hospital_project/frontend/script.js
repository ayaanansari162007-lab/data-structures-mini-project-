document.addEventListener("DOMContentLoaded", () => {
    // Run dashboard code only when dashboard elements exist.
    if (document.getElementById("totalDoctors")) {
        updateDashboard();
    }
});

function updateDashboard() {
    const doctors = getStoredRecords("doctors");
    const appointments = getStoredRecords("appointments");
    const emergencies = getStoredRecords("emergencies");

    document.getElementById("totalDoctors").textContent = doctors.length;
    document.getElementById("totalAppointments").textContent = appointments.length;
    document.getElementById("totalEmergency").textContent = emergencies.length;

    const recentPatients = document.getElementById("recentPatients");

    if (!recentPatients) {
        return;
    }

    // Patients are now stored in the C backend, not localStorage.
    fetch("http://127.0.0.1:8080/api/patients")
        .then(response => {
            if (!response.ok) {
                throw new Error("Unable to load patients.");
            }
            return response.json();
        })
        .then(data => {
            const patients = data.patients || [];

            if (patients.length === 0) {
                recentPatients.innerHTML = `
                    <tr>
                        <td colspan="4" class="empty-state">
                            No patients registered yet.
                        </td>
                    </tr>
                `;
                return;
            }

            recentPatients.innerHTML = patients
                .slice(-5)
                .reverse()
                .map(patient => `
                    <tr>
                        <td>#${escapeHTML(patient.id)}</td>
                        <td>${escapeHTML(patient.name)}</td>
                        <td>${escapeHTML(patient.age)}</td>
                        <td>${escapeHTML(patient.disease)}</td>
                    </tr>
                `)
                .join("");
        })
        .catch(error => {
            console.error("Could not load dashboard patients:", error);

            recentPatients.innerHTML = `
                <tr>
                    <td colspan="4" class="empty-state">
                        Unable to load patients. Check that the C backend is running.
                    </td>
                </tr>
            `;
        });
}

function getStoredRecords(key) {
    try {
        const records = JSON.parse(localStorage.getItem(key) || "[]");
        return Array.isArray(records) ? records : [];
    } catch (error) {
        console.error(`Invalid localStorage data for ${key}:`, error);
        return [];
    }
}

function escapeHTML(value) {
    return String(value ?? "").replace(/[&<>"']/g, character => ({
        "&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&#39;"
    })[character]);
}
