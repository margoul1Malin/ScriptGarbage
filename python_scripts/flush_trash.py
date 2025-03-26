#!/usr/bin/env python3
import os
import sys
import shutil
import subprocess
from pathlib import Path

def empty_trash_windows():
    try:
        # Exécute la commande PowerShell pour vider la corbeille
        subprocess.run(["PowerShell.exe", "Clear-RecycleBin", "-Force"], check=True)
        print("La corbeille a été vidée (Windows).")
    except Exception as e:
        print("Erreur lors de la vidange de la corbeille sur Windows :", e)

def empty_trash_unix():
    home = Path.home()
    local_trash_files = home / ".local/share/Trash/files"
    local_trash_info = home / ".local/share/Trash/info"
    user_trash = home / ".Trash"

    if local_trash_files.exists():
        for item in local_trash_files.glob("*"):
            try:
                if item.is_dir():
                    shutil.rmtree(item)
                else:
                    item.unlink()
            except Exception as e:
                print(f"Erreur lors de la suppression de {item} :", e)
        # Nettoyer le dossier info
        if local_trash_info.exists():
            for item in local_trash_info.glob("*"):
                try:
                    if item.is_dir():
                        shutil.rmtree(item)
                    else:
                        item.unlink()
                except Exception as e:
                    print(f"Erreur lors de la suppression de {item} :", e)
        print("Corbeille vidée : ~/.local/share/Trash")
    elif user_trash.exists():
        for item in user_trash.glob("*"):
            try:
                if item.is_dir():
                    shutil.rmtree(item)
                else:
                    item.unlink()
            except Exception as e:
                print(f"Erreur lors de la suppression de {item} :", e)
        print("Corbeille vidée : ~/.Trash")
    else:
        print("Aucun dossier de corbeille trouvé.")

def main():
    if sys.platform.startswith("win"):
        empty_trash_windows()
    elif sys.platform.startswith("linux") or sys.platform.startswith("darwin"):
        empty_trash_unix()
    else:
        print("Système non supporté.")

if __name__ == "__main__":
    main()
