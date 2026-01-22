### Disclaimer ### 

This project is for educational purposes only. Under no circumstances should it be used to perform malicious actions on a system you do not own.


### Description ### 

This project aims to create a minimalist rootkit. Here are the different steps:
- [x]  **Understanding the `.so` File Structure:** Successfully compile a simple C file into a shared library (`.so`).
- [x]  **Understanding `LD_PRELOAD`:** Write a simple `main` function in a C executable, then write a small function in the `.so` that executes **before** the `main` using the `__attribute__((constructor))` attribute. Validate the loading.
- [x]  **Symbol Management:** Use the **`dlsym`** function with the special **`RTLD_NEXT`** argument to obtain the pointer to the **real** function from the standard library (for example, the real `puts` from `libc`).
- [x]  **Define the Target Function:** Identify the standard function to intercept for filtering `/proc` content. The **`readdir`** function (which reads directory entries) is the most relevant for hiding PID directories.
- [x]  **Create the Overloaded Function (the Hook):** Write your own version of `readdir` that has exactly the same signature. It must call the **original** (the real `readdir` obtained in step 3) to get an entry.
- [x]  **Test Static Filtering:** Implement the logic in your overloaded `readdir` to check if the directory entry is a **predefined static PID** (e.g., `5000`). If so, ignore this entry by looping and calling the original again.
- [x]  **Proof of Concept Test:** Load your `.so` with `LD_PRELOAD` on a simple directory listing tool (`ls /proc`). Validate that the static PID directory (e.g., `/proc/5000`) is indeed invisible.
- [x]  **Retrieve the PID to Hide:** Modify the `constructor` function (step 2) to retrieve the PID of the process to hide. A simple and robust technique is to read this PID from an **environment variable** (e.g., `HIDE_ME_PID`).
- [x]  **Dynamic Filtering:** Modify the logic of the overloaded `readdir` function to compare the read entry (`dirp->d_name`) with the **dynamic** PID retrieved in step 8.
- [x]  **Complete Rootkit Test:** Create a small "launcher" program in C (or a shell script) that: 1) Launches the **target** process (the one you want to hide) in the background and notes its PID. 2) Sets the `HIDE_ME_PID` variable with this PID. 3) Executes the monitoring tool (`ps`, `top`) with `LD_PRELOAD` pointing to your `.so`. Validate that the PID does not appear.
- [x]  **Permission Management:** Add a simple verification so that your hook only activates if a certain "password" or secret variable is present in the environment. This prevents the hook from accidentally activating on all programs.
- [x]  **Environment Cleanup:** After reading the PID to hide, use **`unsetenv`** to remove the environment variable (`HIDE_ME_PID`) from the process memory. This adds a layer of stealth so that an attacker cannot find the hidden PID by examining environment variables.


### Usage ### 

To test the rootkit:
- Compile the program using `make`
- In a terminal, run the following command (to hide PID 5000):
```bash
ROOTKIT_PWD=password123 HIDE_ME_PID=5000 LD_PRELOAD=./hook.so ls /proc
```

### Features ###

- ✅ Function hooking using `LD_PRELOAD`
- ✅ Dynamic PID hiding via environment variable
- ✅ Password protection to prevent accidental activation
- ✅ Environment variable cleanup for stealth
- ✅ Intercepts `readdir` to filter `/proc` entries