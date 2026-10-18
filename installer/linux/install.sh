#!/bin/sh
# Installs Forge for this user on Linux: into ~/.local/share/forge, with the frg command in ~/.local/bin.
# Run it from the unpacked Forge folder:  ./install.sh
# Installing again (a newer Forge) replaces the old one; your games are never touched, they live in their own folders.
set -e

here="$(cd "$(dirname "$0")" && pwd)"
dest="$HOME/.local/share/forge"
bindir="$HOME/.local/bin"

if [ ! -x "$here/bin/frg" ]; then
    echo "error: run install.sh from the unpacked Forge folder (the one with bin/frg in it)"
    exit 1
fi

echo "Installing Forge into $dest"
rm -rf "$dest"
mkdir -p "$dest" "$bindir"
cp -R "$here/." "$dest/"
ln -sf "$dest/bin/frg" "$bindir/frg"
echo "    frg is $bindir/frg"

# Forge Edit, the light code editor: as a command, and in the apps menu
if [ -x "$dest/bin/forge-edit" ]; then
    ln -sf "$dest/bin/forge-edit" "$bindir/forge-edit"
    apps="$HOME/.local/share/applications"
    mkdir -p "$apps"
    cat > "$apps/forge-edit.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Forge Edit
Comment=A light code editor for Forge games
Exec=$dest/bin/forge-edit %F
Icon=$dest/art/forge-icon-512.png
Terminal=false
Categories=Development;IDE;
EOF
    echo "    Forge Edit is in your apps menu (or run: forge-edit)"
fi

# The VS Code extension (colors, errors as you save, a Run button), if VS Code is installed.
if command -v code > /dev/null 2>&1 && [ -f "$dest/editors/forge.vsix" ]; then
    code --install-extension "$dest/editors/forge.vsix" --force > /dev/null 2>&1 &&
        echo "    the Forge extension is installed in VS Code"
fi

# What Forge needs from the system: g++ to build games, and the OpenGL and X11 headers for games with a window.
missing=""
command -v g++ > /dev/null 2>&1 || missing="$missing g++"
[ -f /usr/include/GL/gl.h ] || missing="$missing libgl1-mesa-dev"
[ -f /usr/include/X11/Xlib.h ] || missing="$missing libx11-dev"
if [ -n "$missing" ]; then
    echo ""
    echo "Forge also needs a few things from Ubuntu. Install them with (it asks for your password):"
    echo "    sudo apt install$missing"
fi

case ":$PATH:" in
    *":$bindir:"*) ;;
    *)
        echo ""
        echo "$bindir isn't on your PATH yet. Log out and back in (Ubuntu adds it when it exists),"
        echo "or add this line to the end of ~/.bashrc and open a new terminal:"
        echo "    export PATH=\"\$HOME/.local/bin:\$PATH\""
        ;;
esac

echo ""
echo "Done. Check it with:  frg version"
echo "Then make a game:     frg new mygame && cd mygame && frg run"
