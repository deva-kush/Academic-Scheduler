from __future__ import annotations

import csv
import os
import platform
import subprocess
from pathlib import Path

from flask import Flask, jsonify, request, send_file

BASE_DIR = Path(__file__).resolve().parent
EXECUTABLE_NAME = "main.exe" if os.name == "nt" else "main"
CPP_EXECUTABLE = BASE_DIR / "build" / EXECUTABLE_NAME
RELEASE_CPP_EXECUTABLE = (
    BASE_DIR / "build" / "Release" / EXECUTABLE_NAME
)
DEBUG_CPP_EXECUTABLE = BASE_DIR / "build" / "Debug" / EXECUTABLE_NAME
LEGACY_CPP_EXECUTABLE = BASE_DIR / EXECUTABLE_NAME

app = Flask(__name__)


# ---------------------------------------------------------------------------
# Storage helpers
# ---------------------------------------------------------------------------
def data_path(filename: str) -> Path:
    """Resolve all application CSV files relative to the application."""
    return BASE_DIR / filename


def append_csv(filename: str, row: list) -> None:
    path = data_path(filename)

    with path.open("a", newline="", encoding="utf-8") as file:
        csv.writer(file).writerow(row)


def read_csv(filename: str) -> list[list[str]]:
    path = data_path(filename)

    if not path.exists():
        return []

    with path.open("r", newline="", encoding="utf-8") as file:
        return list(csv.reader(file))


def require_json_fields(data: dict | None, fields: tuple[str, ...]) -> None:
    if not isinstance(data, dict):
        raise ValueError("Request body must be JSON.")

    missing = [field for field in fields if field not in data]
    if missing:
        raise ValueError(
            "Missing required fields: " + ", ".join(missing)
        )


# ---------------------------------------------------------------------------
# Frontend
# ---------------------------------------------------------------------------
@app.get("/")
def home():
    return send_file(BASE_DIR / "index.html")


# ---------------------------------------------------------------------------
# Create endpoints
# ---------------------------------------------------------------------------
@app.post("/add_room")
def add_room():
    try:
        data = request.get_json(silent=True)
        require_json_fields(
            data, ("id", "capacity", "building")
        )

        append_csv(
            "rooms.csv",
            [
                data["id"],
                data["capacity"],
                data["building"],
                data.get("x", 0),
                data.get("y", 0),
            ],
        )

        return jsonify({"msg": "Room added"}), 201

    except ValueError as exc:
        return jsonify({"error": str(exc)}), 400


@app.post("/add_exam")
def add_exam():
    try:
        data = request.get_json(silent=True)
        require_json_fields(
            data, ("id", "courseId", "date", "priority")
        )

        append_csv(
            "exams.csv",
            [
                data["id"],
                data["courseId"],
                data["date"],
                data["priority"],
            ],
        )

        return jsonify({"msg": "Exam added"}), 201

    except ValueError as exc:
        return jsonify({"error": str(exc)}), 400


@app.post("/add_event")
def add_event():
    try:
        data = request.get_json(silent=True)
        require_json_fields(
            data, ("id", "title", "participants", "priority")
        )

        append_csv(
            "events.csv",
            [
                data["id"],
                data["title"],
                data["participants"],
                data["priority"],
            ],
        )

        return jsonify({"msg": "Event added"}), 201

    except ValueError as exc:
        return jsonify({"error": str(exc)}), 400


@app.post("/add_course")
def add_course():
    try:
        data = request.get_json(silent=True)
        require_json_fields(
            data, ("id", "name", "students")
        )

        append_csv(
            "courses.csv",
            [
                data["id"],
                data["name"],
                data["students"],
                data.get("dept", "GEN"),
                data.get("occurrences", 1),
            ],
        )

        return jsonify({"msg": "Course added"}), 201

    except ValueError as exc:
        return jsonify({"error": str(exc)}), 400


@app.post("/add_registration")
def add_registration():
    try:
        data = request.get_json(silent=True)
        require_json_fields(data, ("student", "course"))

        student_id = int(data["student"])
        course_id = int(data["course"])

        courses = read_csv("courses.csv")
        course_ids = {
            int(row[0])
            for row in courses
            if row and row[0].strip().lstrip("-").isdigit()
        }

        if course_id not in course_ids:
            return jsonify({
                "error": f"Course ID {course_id} not found"
            }), 400

        append_csv("regsub.csv", [student_id, course_id])

        return jsonify({
            "msg": (
                f"Student {student_id} registered "
                f"for course {course_id}"
            )
        }), 201

    except (ValueError, TypeError) as exc:
        return jsonify({"error": str(exc)}), 400


# ---------------------------------------------------------------------------
# Read endpoints
# ---------------------------------------------------------------------------
@app.get("/get_rooms")
def get_rooms():
    return jsonify(read_csv("rooms.csv"))


@app.get("/get_exams")
def get_exams():
    return jsonify(read_csv("exams.csv"))


@app.get("/get_events")
def get_events():
    return jsonify(read_csv("events.csv"))


@app.get("/get_courses")
def get_courses():
    return jsonify(read_csv("courses.csv"))


@app.get("/get_registrations")
def get_registrations():
    return jsonify(read_csv("regsub.csv"))


# ---------------------------------------------------------------------------
# C++ scheduler execution
# ---------------------------------------------------------------------------
def run_scheduler(choice: int) -> tuple[str, str, int]:
    executable = next(
        (
            candidate
            for candidate in (
                CPP_EXECUTABLE,
                RELEASE_CPP_EXECUTABLE,
                DEBUG_CPP_EXECUTABLE,
                LEGACY_CPP_EXECUTABLE,
            )
            if candidate.exists()
        ),
        None,
    )

    if executable is None:
        raise FileNotFoundError(
            f"Scheduler executable not found: {EXECUTABLE_NAME}. "
            "Build the C++ project with `cmake -S . -B build` and "
            "`cmake --build build`."
        )

    result = subprocess.run(
        [str(executable)],
        input=f"{choice}\n",
        capture_output=True,
        text=True,
        cwd=BASE_DIR,
        timeout=20,
        check=False,
    )

    return result.stdout, result.stderr, result.returncode


@app.get("/generate/<int:choice>")
def generate(choice: int):
    if choice not in {2, 3, 4, 5}:
        return jsonify({
            "error": "Invalid choice. Supported modes: 2, 3, 4, 5."
        }), 400

    try:
        stdout, stderr, return_code = run_scheduler(choice)

        if return_code != 0:
            return jsonify({
                "error": "Scheduler failed.",
                "stdout": stdout,
                "stderr": stderr,
                "returncode": return_code,
            }), 500

        if not stdout.strip():
            return jsonify({
                "error": "Scheduler returned no output.",
                "stderr": stderr,
            }), 500

        return jsonify({
            "output": stdout,
            "stderr": stderr,
            "mode": choice,
        })

    except subprocess.TimeoutExpired:
        return jsonify({
            "error": "Scheduler timed out."
        }), 504

    except FileNotFoundError as exc:
        return jsonify({"error": str(exc)}), 500

    except Exception as exc:
        app.logger.exception("Scheduler execution failed")
        return jsonify({"error": "Internal server error"}), 500


# ---------------------------------------------------------------------------
# Development entry point
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    # Development only. For production we will use a WSGI server.
    app.run(host="127.0.0.1", port=5000, debug=False)
