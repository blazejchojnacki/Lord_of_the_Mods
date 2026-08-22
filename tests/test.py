import os
import shutil
from pathlib import Path

# Import your compiled C++ module
# (Ensure you have run `pip install .` so pybind11 has built the .pyd file)
import lotm

def create_mock_file(path: Path, content: str):
    """Utility to create directories and write a text file."""
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content)

def print_file_status(path: Path):
    """Utility to safely read and print file contents."""
    if path.exists():
        print(f"[EXISTS] {path.name} -> Content: {path.read_text()}")
    else:
        print(f"[MISSING] {path.name}")

def main():
    print("======================================")
    print("  Python Mod Installer Sandbox Test   ")
    print("======================================\n")

    # 1. Setup Sandbox Paths using modern pathlib
    sandbox = Path("./python_sandbox")
    game_dir = sandbox / "game"
    backup_dir = sandbox / "backup"
    mod_dir = sandbox / "mods" / "elven_mod"

    # Clean up previous runs to ensure a fresh test environment
    if sandbox.exists():
        shutil.rmtree(sandbox)

    # 2. Generate Mock Files
    print("--- STEP 1: Setting up mock files ---")
    target_file = Path("data/ini/gamedata.ini")
    create_mock_file(game_dir / target_file, "VANILLA_DATA")
    create_mock_file(mod_dir / target_file, "MODDED_DATA")
    
    print("Original Game File:")
    print_file_status(game_dir / target_file)

    # 3. Initialize C++ Objects via Python Bindings
    print("\n--- STEP 2: Initializing C++ Core ---")
    
    # Instantiate the C++ struct and set its properties
    file_data = lotm.FileData()
    file_data.relative_path = str(target_file)
    file_data.hash = "dummy_hash_123"
    file_data.transfer_type = lotm.TransferType.COPY

    # Instantiate the C++ Manifest and Installer
    test_mod = lotm.ModManifest("py_mod_01", "Python Test Mod", str(mod_dir))
    test_mod.add_file(file_data)

    installer = lotm.ModInstaller(str(game_dir), str(backup_dir))
    print("C++ Core initialized successfully.")

    # 4. Test Activation
    print("\n--- STEP 3: Activating Mod ---")
    success = installer.activate(test_mod)
    print(f"Activation Result: {'SUCCESS' if success else 'FAILED'}")
    
    print("\nFile States Post-Activation:")
    print(f"Game:   ", end=""); print_file_status(game_dir / target_file)
    print(f"Mod:    ", end=""); print_file_status(mod_dir / target_file)
    print(f"Backup: ", end=""); print_file_status(backup_dir / target_file)

    # 5. Test Deactivation
    print("\n--- STEP 4: Deactivating Mod ---")
    success = installer.deactivate(test_mod)
    print(f"Deactivation Result: {'SUCCESS' if success else 'FAILED'}")

    print("\nFile States Post-Deactivation:")
    print(f"Game:   ", end=""); print_file_status(game_dir / target_file)
    print(f"Backup: ", end=""); print_file_status(backup_dir / target_file)

# Standard Python execution block
if __name__ == "__main__":
    main()
