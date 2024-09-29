# Tiny Pomodoro

A dirt-simple console Pomodoro timer that tracks the total time spent working each day. It can also play a sound when the timer finishes using the `bass.dll` library.

![](/docs/1.png)
![](/docs/2.png)

## Features

- **Simple Timer**: The default timer is set to 25 minutes.
- **Customizable Timer**: You can specify the number of minutes via a command-line argument.
- **Total Time Logging**: The timer logs the total work time for each day in a text file inside a `log` folder.
- **Optional Logging**: You can disable logging by passing a `-nolog` argument.
- **Sound Notification**: Plays a notification sound when the timer is up.

## Usage

### Basic Usage

To run the timer with the default 25 minutes:

```bash
TinyPomodoro.exe
```

### Custom Time

To run the timer with a custom amount of minutes, use the `-m` option followed by the number of minutes:

```bash
TinyPomodoro.exe -m 50
```

This will start the timer for 50 minutes.

### Disabling Logging

To run the timer without logging the elapsed time:

```bash
TinyPomodoro.exe -nolog
```

Or, you can combine both options to specify a custom time and disable logging:

```bash
TinyPomodoro.exe -m 40 -nolog
```

### File Logging

If logging is enabled, the program will create a `log` folder and log the total work time for each day in a file named `DD-MM-YYYY.txt`, where `DD-MM-YYYY` is the current date. The file will contain the total amount of work time in hours for that day. If the timer is run multiple times on the same day, the new time will be accumulated and saved to the same file.

For example, running the timer twice on `23-09-2024` for 25 minutes each will result in the following content in the file `log/23-9-2024.txt`:

```
0.833333 hour(s).
```

## Example

1. Run the timer for the default 25 minutes:

```bash
TinyPomodoro.exe
```

2. Run the timer for 45 minutes:

```bash
TinyPomodoro.exe -m 45
```

3. Run the timer for 30 minutes without logging:

```bash
TinyPomodoro.exe -m 30 -nolog
```

4. Check the `log` folder to see the total work time for each day.
