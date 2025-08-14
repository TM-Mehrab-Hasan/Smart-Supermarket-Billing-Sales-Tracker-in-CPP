# BillMaster++ 🛒

**BillMaster++** is a full-featured offline **Supermarket POS and Billing System** built in C++.  
It supports inventory management, customer billing, receipts, low-stock alerts, and daily sales reports with visual charts — all designed for Windows offline use.


**Next-Level Supermarket POS & Billing System in C++**  

![C++](https://img.shields.io/badge/Language-C++-blue)
![Offline](https://img.shields.io/badge/Offline-100%25-green)
![License](https://img.shields.io/badge/License-MIT-yellow)
![Build](https://img.shields.io/badge/Build-Passing-brightgreen)

---

## Features

- **Add / Update Items**
  - Manage item name, rate, and stock quantity.
- **Print Bills / Sell Items**
  - Table-style receipts with proper alignment and borders.
  - Automatic VAT (5%) and discount calculation.
- **Inventory Display & Search**
  - View all items or search by keyword.
- **Low Stock Alerts**
  - Warns when stock of any item falls below a threshold (configurable).
- **Sales Tracking**
  - Every transaction is saved in `Sales.txt`.
- **Daily Report**
  - Customer-wise sales summary.
  - Item-wise quantity sold.
  - Simple ASCII bar charts for sales visualization.
  - Total revenue calculation.
- **Automatic Receipt Generation**
  - Receipts saved in `Receipts/` folder.
- **Reports Folder**
  - Daily report saved in `Reports/` folder.

---

## Folder Structure

```text
BillMaster++
│
├─ BillMaster.cpp         # Main C++ POS code
├─ README.md              # Project overview & GIF demo
├─ LICENSE                # MIT License
├─ Bill.txt               # Sample inventory file
├─ Sales.txt              # Sales records
├─ Receipts/              # Generated receipts
├─ Reports/               # Daily sales reports
└─ GIFs/                  # Optional demo GIFs

```

## ⚙️ Quick Start / Demo

### 1️⃣ Windows Setup (Demo-Ready)

I've provided a **batch script** `setup_demo.bat` for easy setup:

```bat
@echo off
echo Compiling BillMaster.cpp...
g++ -o BillMaster.exe BillMaster.cpp

IF %ERRORLEVEL% NEQ 0 (
    echo Compilation failed! Make sure g++ is installed.
    pause
    exit /b
)
echo Compilation successful!

IF NOT EXIST Receipts mkdir Receipts
IF NOT EXIST Reports mkdir Reports
IF NOT EXIST GIFs mkdir GIFs

(
echo Apple|50|20
echo Banana|30|30
echo Orange|40|25
echo Milk|60|15
echo Bread|25|40
) > Bill.txt

IF NOT EXIST Sales.txt echo. > Sales.txt

echo Running BillMaster++ Demo...
pause
BillMaster.exe
echo Demo finished!
pause

```

### 2️⃣ How to Run

1. Clone the repository.
2. Open Windows Command Prompt in the repo folder.
3. Run: setup_demo.bat
4. Follow on-screen menu instructions to add items, sell, and generate reports.
5. Check Receipts/ and Reports/ for generated files.

## Usage

1. Run BillMaster++.exe.

2. Main menu options:

   - 1: Add or update item

   - 2: Print bill / sell items

   - 3: Display full inventory

   - 4: Search inventory by name

   - 5: Generate daily report

   - 6: Exit program

3. For each sale:

  - Enter customer name.

  - Add items and quantity.

  - Bill will be calculated with VAT & discount.

  - Receipt automatically saved in Receipts/.

4. Daily reports:

  - Generated in Reports/ folder.

  - Includes customer summary, item sales, and total revenue.

## Screenshots (Console Example)

```text
+----------------+-------+-------+--------+
| Item           | Rate  | Qty   | Amount |
+----------------+-------+-------+--------+
| Milk           |   50  |     2 |    100 |
| Bread          |   30  |     1 |     30 |
+----------------+-------+-------+--------+
Total: 130 BDT
VAT(5%): 6.50 BDT
Discount: 0 BDT
Net Total: 136.50 BDT

```

## 💡 Why BillMaster++

- Professional receipt formatting in console
- Visual insights with ASCII charts
- Lightweight and fully offline
- Perfect for learning C++ file handling and console applications
- Fully demo-ready with preloaded inventory

## 📄 License

This project is licensed under the MIT License – see LICENSE for details.
