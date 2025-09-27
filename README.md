# Word Frequency Count – Naive vs Multiprocessing vs Multithreading

## 📌 Overview
This project was developed as part of a **university course assignment**.  
It implements a **word frequency counter** in C using three different approaches:

1. **Naive (Sequential)** – `Naive.c`  
2. **Multiprocessing** – `MultiProcessing.c`  
3. **Multithreading** – `Threading.c`  

The goal is to **compare execution times** of these approaches and evaluate their performance for large-scale word frequency counting tasks.

---

## 📂 Project Structure
uni-systems-wordfreq-comparison/
├── Naive.c # Sequential implementation
├── MultiProcessing.c # Multiprocessing implementation
├── Threading.c # Multithreading implementation
├── OSReport.pdf # Detailed report with results and analysis
└── README.md # Project documentation


---

## ⚙️ Approaches
- **Naive:** Executes sequentially, easiest but slowest.  
- **Multiprocessing:** Splits the work across multiple processes for true parallelism.  
- **Multithreading:** Uses POSIX threads (pthreads) to run tasks concurrently with synchronization.  

---

## 🚀 How to Compile & Run
Make sure you have **GCC** and **POSIX threads** installed (Linux or WSL recommended).

### Compile:
```bash
# Naive approach
gcc Naive.c -o naive

# Multiprocessing approach
gcc MultiProcessing.c -o multiprocessing

---
📊 Results

Execution time and throughput were measured for each approach.

Naive: Slowest, suitable only for small files.
Multiprocessing: Best speedup using multiple CPU cores.
Multithreading: Moderate improvement, good for I/O-bound tasks.

📑 Detailed results, figures, and Amdahl’s law analysis are in OSReport.pdf.

# Multithreading approach (requires -lpthread)
gcc Threading.c -o threading -lpthread

👩‍🎓 Author : Rawand Radi

Faculty of Engineering and Technology-Birzeit university
