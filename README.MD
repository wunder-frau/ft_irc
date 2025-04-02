### **📌 How to Run the IRC Server, `irssi`, and Your Custom Client**

This guide explains how to **start the IRC server, connect with `irssi`, and test your custom client**.

## **🚀 Step 1: Start the IRC Server (`ngIRCd`)**

Make sure **ngIRCd** is installed. If not, install it:

- **Ubuntu/Debian:**
  ```sh
  sudo apt install ngircd -y
  ```
- **macOS (Homebrew):**
  ```sh
  brew install ngircd
  ```

### **Start the server:**

```sh
ngircd
```

Verify the server is running on port `6667`:

```sh
netstat -an | grep 6667
```

Expected output:

```
tcp4       0      0  *.6667                 *.*                    LISTEN
```

---

## **🚀 Step 2: Connect with `irssi` (for monitoring)**

1. Open a new terminal and start `irssi`:
   ```sh
   irssi
   ```
2. Connect to the local IRC server:
   ```sh
   /connect 127.0.0.1 6667
   ```
3. Join a test channel:
   ```sh
   /join #testchannel
   ```

---

## **🚀 Step 3: Start Your Custom IRC Client**

1. Open another terminal and navigate to your client directory:
   ```sh
   cd /path/to/your/client
   ```
2. Compile the client (if not already compiled):
   ```sh
   g++ -o client Client.cpp main.cpp -std=c++17 -Wall -Wextra -pthread
   ```
3. Run the client:
   ```sh
   ./client
   ```
4. Inside the client, join the test channel:
   ```
   JOIN #testchannel
   ```
   ✅ Expected output in `irssi`:
   ```
   *** TestUser has joined #testchannel
   ```

---

## **🚀 Step 4: Send Test Messages**

### **🔹 From the Client to `irssi`**

Inside the client, send a test message:

```
PRIVMSG #testchannel :Hello from my IRC client!
```

✅ Expected output in `irssi`:

```
<TestUser>: Hello from my IRC client!
```

### **🔹 From `irssi` to the Client**

Inside `irssi`, send a message:

```sh
/msg TestUser Hello from irssi!
```

✅ Expected output in the client:

```
<Server>: :iresta!~iresta@localhost PRIVMSG TestUser :Hello from irssi!
```

---

## **🚀 Step 5: Verify the Client Stays Connected**

Wait for **30-60 seconds** to confirm your client does **not** time out.  
If it does, ensure **PING-PONG handling** is implemented in `Client.cpp`.

---

## **🚀 Step 6: Cleanly Exit**

- **To disconnect the client gracefully**, type:
  ```
  /quit
  ```
  ✅ Expected output in `irssi`:
  ```
  *** TestUser has quit (Goodbye!)
  ```
- **To stop `irssi`**, type:
  ```
  /quit
  ```
- **To stop the IRC server (`ngircd`)**, run:
  ```sh
  pkill ngircd
  ```
