# Simple P2P Messenger

**Note:** This project is abandoned. Please use [QuickChat](https://github.com/WinXP655/quickchat)

### What is this?
P2P Messenger using pure Win32. Chatting directly by using LAN or Wi-Fi. Works on Windows 2000 and higher.

---

### Features

* Only Win32, no 3rd party dependencies.
* Works both in server and client mode.
* Server saves chat history to `chatlog.txt`.
* Sound notifications about new messages and user join.
* Restart and end session using "Session" menu.
* Minimalistic UI.

---

### How to run

#### On server (Computer 1):

1. Run app.
2. Click "Yes" in message box.
3. If you see your IP - send it to friend to join chat.
4. Allow network access in Windows Firewall.
5. Wait while client will connect.
6. Start chatting.

#### On client (Computer 2):

1. Run app.
2. Click "No" in message box.
3. Enter server IP.
4. Allow network access in Windows Firewall.
5. Chat after connection.

---

### Requirements

* LAN or Wi-Fi.
* Windows 2000 and newer.
* App should be allowed in Windows Firewall.

---

### How to use

* Type your message in bottom text edit, press "Send" or Enter (without Shift) - send message.
* Shift+Enter - new line.
* Users list on left side (You and Remote PC).
* Message history.
* "Session" menu - Restart or End session.
* "Help" menu - app info.

---

### Logs

* Server saves all chat history to `chatlog.txt`.

### Changelog

**Chat2 (2 June 2025)**:
- P2P Messenger renamed to Chat2.
- Removed error sound.
- Deleted redunant copy of About dialog.
- Deleted not needed comments and reduced length of others.
- Added code for incoming/outcoming connection warning dialog (with P2P Messenger title - originally was planned for previous product).
- Removed code for DPI aware fonts.
- Removed Lucida Console usage.
- Main icon updated to Windows 7 style.
- Last version with installer package.
- System sound was present in code, but sound file do not exists.
- Problem: Font became too small because font size is static positive.
Code is from 2 June 2026, but binary is from 10 June 2026 - I didn't wanted to continue P2P Messenger family, but needed to release something.

**P2P Messenger 2.1 Beta (31 May 2025)**:
- Added chat saving/clearing option.
- Renamed msg.wav to newmsg.wav.
- Renamed sys.wav to system.wav.
- Added trimming whitespaces, validating IP format in server connection dialog.
- Added error handling in client-side code.
- Added graceful chat leave.
- Added a prototype of incoming/outcoming connection warning dialog without code (will be added later).**

**P2P Messenger 2.0 (Date is lost)**:
- Replaced wsock32.dll with ws2_32.dll while using same WinSock 1.1.

**P2P Messenger 1.5 (Date is list)**:
- Added sounds.
- Added Enter key handler.
- Added chat logging.
- Added menu bar.
- Added about dialog.
- Added restart option.
- Added server IP dialog.
- Using Lucida Console and Tahoma instead raster System.
- Added 1 pixel border for messages list.
- Added join/leave message.
- Added custom fixed colors for users and message list.
- Added trimming whitespaces and newlines.
- Added window flashing.
- Added hidden SYSTEM user (message tag: [SYSTEM]).
- Implemented Installer Package (used NSIS), but code still mostly portable.

**P2P Messenger 1.2 (Date is unknown)**:
- Replaced messages list box with EDIT control.

**P2P Messenger 1.1 (27 May 2025)**:
- Added users list.
- Added user names by computer name.
- Added user name exchange (required).

**P2P Messenger 1.0g (Date is unknown)**:
- Removed console output.
- Added GUI.
- Added IP typing dialog.

**P2P Messenger 1.0 (24 May 2025)**:
- Initial primitive prototype: Console Window, TCP Pipe, Server and Client Modes, Messages Receiver.
- No error handling except network error.

---

Enjoy!
If you have any questons - write me on Discord - @pcsettings
