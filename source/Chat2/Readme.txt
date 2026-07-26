Simple P2P Messenger  
====================

Author: WinXP655 (2025)

Description:
------------
This is a simple P2P messenger written in pure Win32 API.  
It allows you to chat directly with another computer — no server required.

Features:
---------
- Minimalistic user interface using native WinAPI. No third-party dependencies.
- Works in both server and client modes.
- Logs chat history to "chatlog.txt" (server-side only).
- Sound notifications for new messages and connections.
- Ability to restart or end the session via the menu.
- Supported on Windows 2000 and newer.
- Encrypting messages for security.

How to Run:
-----------

Computer 1 (Server):
1. Launch the program.
2. Select server mode (click "Yes" in the message box).
3. The app will display your IP address — share it with the other user.
4. If Windows Firewall prompts you, click "Allow access".
5. Wait for the incoming connection.
6. Once connected, start chatting.

Computer 2 (Client):
1. Launch the program.
2. Select client mode (click "No" in the message box).
3. Enter the server's IP address and click "Connect".
4. If Windows Firewall prompts you, click "Allow access".
5. Once connected, start chatting.

Requirements:
-------------
- Stable network connection (LAN or Wi-Fi).
- Windows 2000 or newer.
- Messenger must be allowed in Windows Firewall.
Both computers should be in the same local network or have direct network access (no firewall blocking).

Troubleshooting:
----------------
- If connection fails, check your internet settings and firewall rules.
- A stable network connection is required for reliable communication.
- This program supports 1:1 chat only — no group chat functionality.

How to Use:
-----------
- Type your message in the lower text box.
- Click "Send" or press Enter (without Shift) to send.
- Press Shift+Enter for a new line.
- The user list on the left shows both you and your peer.
- The upper text area displays the chat history.
- Use the "Session" menu to restart or end the session.
- The "Help" menu contains program info.

Logs:
-----
- The server saves the full chat log to "chatlog.txt" in the program folder.

Thanks for using Simple P2P Messenger!
--------------------------------------

