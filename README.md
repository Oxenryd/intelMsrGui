# IntelMsrGui

**IntelMsrGui** is a small GUI tool I built for myself to experiment with and control **Intel CPU MSRs**, mainly for **voltage tuning** on **Skylake and newer Intel CPUs**.

![alt text](https://www.dropbox.com/scl/fi/453g7jod0j37wusxaqqw7/Sk-rmbild_20260105_181522.png?rlkey=bvqgvp2q5bs8akje7w646j8q3&st=8n0d0k1t&dl=0 "IntelMsrGui")

It’s an evolution of an earlier command-line project of mine (**Intel-MSR-Tool**), where I wanted something more visual and a bit more convenient to use day-to-day.
The UI is built with **Qt 6**.

This repo is public mostly so others can **browse the code**, not because it’s meant to be a polished or “safe” utility.

---

## What it is (and isn’t)

This is:

* A **personal tool** I use on my own systems
* A way for me to explore **low-level CPU control** with a GUI on top
* A place where convenience sometimes wins over perfect architecture

This is not:

* A finished product
* A general-purpose utility
* Something I’d recommend running blindly on random hardware

---

## Notes on the current state

Right now:

* MSR access is **not cleanly separated** from the GUI
* The whole application needs **elevated permissions** to work
* That’s obviously not ideal, but good enough for my own use for now

If this were to be taken further, the first real step would be to:

* Move all MSR logic back into a small, isolated CLI or service
* Let the GUI talk to that instead of touching MSRs directly

I just haven’t bothered doing that yet.

---

## A quick warning

This tool:

* Can mess with CPU voltages
* Needs high privileges
* Can absolutely do the wrong thing if misused

I use it on my own machines, I understand the risks, and I’m fine with that.
If you run it yourself, you’re on your own.

---

## Why this repo is public

Mostly for **code inspection**.

Expect rough edges and unfinished ideas.

---

That’s all it’s meant to be. Thanks! =)
