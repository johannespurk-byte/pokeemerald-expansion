import os

# Sucht ausgehend vom aktuellen Standort des Skripts nach dem Grafikordner
current_dir = os.path.dirname(os.path.abspath(__file__))
target_dir = os.path.normpath(os.path.join(current_dir, "..", "graphics", "items", "icons"))

pal_lines = [
    b"JASC-PAL",
    b"0100",
    b"16"
]
for _ in range(16):
    pal_lines.append(b"0 0 0")

valid_pal_content = b"\r\n".join(pal_lines) + b"\r\n"

if not os.path.exists(target_dir):
    # Falls das Skript im Hauptordner liegt, probieren wir den direkten Pfad
    target_dir = os.path.normpath(os.path.join(current_dir, "graphics", "items", "icons"))

if not os.path.exists(target_dir):
    print(f"Error: Could not automatically find the icons folder! Checked: {target_dir}")
else:
    count = 0
    for filename in os.listdir(target_dir):
        if filename.endswith(".pal"):
            file_path = os.path.join(target_dir, filename)
            with open(file_path, "wb") as f:
                f.write(valid_pal_content)
            count += 1
            
    print(f"Success: {count} palette files in the true folder ({target_dir}) repaired successfully!")
