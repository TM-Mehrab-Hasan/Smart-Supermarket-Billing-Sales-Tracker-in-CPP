# Bill Master++ 🛒

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

## 📸 System Screenshots  

### 1. Main Menu  
![Main Menu](System%20Visuals/Main_Menu.png)  

### 2. Inventory Management 
![Inventory Management](System%20Visuals/Inventory_Management.png)  

### 3. Inventory 
![Inventory](System%20Visuals/Inventory.png)

### 4. Search Item in Inventory [Not Case Sensitive]
![Search Inventory](System%20Visuals/Search_Item.png)  

### 5. Daily Report
![Daily Report](System%20Visuals/Daily_Report.png)  

### 6. Sales History
![Sales History](System%20Visuals/Sales_History.png)  

---

## How to  run?

1. Clone the repository
2. Run the BillMaster.exe file and enjoy!

---

## 🏗️ **Project Root Directory**
```
BillMaster++/
│
├── 📄 Source Code Files
│   ├── BillMaster.cpp              # Main C++ source code (your enhanced version)
│   ├── setup_demo.bat              # Windows setup script
│   └── README.md                   # Project documentation
│
├── 🔧 Compiled Files
│   ├── BillMaster.exe              # Windows executable (after compilation)
│
├── 💾 Data Files (Created automatically)
│   ├── Bill.txt                    # Inventory database
│   ├── Sales.txt                   # Sales transaction log
│   └── README.txt                  # Auto-generated user guide
│
├── 📂 Generated Folders (Created by program/script)
│   ├── Receipts/                   # Individual customer receipts
│   │   ├── Receipt_2024-08-14_10-30-15_John_Doe.txt
│   │   ├── Receipt_2024-08-14_11-45-22_Walk-in_Customer.txt
│   │   └── ...
│   │
│   ├── Reports/                    # Daily sales reports
│       ├── DailyReport_2024-08-14.txt
│       ├── DailyReport_2024-08-15.txt
│       └── ...
│
└── 📋 Optional Files
    ├── .gitignore                  # Git ignore file
    ├── LICENSE.txt                 # License file
    └── CHANGELOG.md                # Version history
```

---

## 📄 **File Descriptions**

### **Core Source Files**
| File | Description | Required |
|------|-------------|----------|
| `BillMaster.cpp` | Main C++ source code with all POS functionality | ✅ Yes |
| `build_and_run.bat` | Automated setup script for Windows | 🔧 Recommended |

### **Data Files (Auto-generated)**
| File | Description | Format | Sample Content |
|------|-------------|--------|----------------|
| `Bill.txt` | Inventory database | `Name\|Rate\|Quantity` | `Apple\|55.50\|25` |
| `Sales.txt` | Sales transaction log | Multi-line format | Transaction records |
| `README.txt` | Auto-generated user guide | Plain text | Usage instructions |

### **Generated Directories**
| Directory | Purpose | Contents |
|-----------|---------|----------|
| `Receipts/` | Customer receipts | Individual `.txt` receipt files |
| `Reports/` | Business reports | Daily sales analysis files |

---

## 🗂️ **Sample File Contents**

### **Bill.txt (Inventory Database)**
```
# Inventory File - Format: Name|Rate|Quantity
Apple|55.50|25
Banana|32.00|40
Orange|48.75|30
Milk 1L|65.00|20
Bread|28.50|35
Rice 1kg|85.00|50
```

### **Sales.txt (Transaction Log)**
```
2024-08-14 10:30:15|John Doe|245.50
Apple|55.50|25|2
Milk 1L|65.00|20|1
Bread|28.50|35|3
---
2024-08-14 11:45:22|Walk-in Customer|120.75
Banana|32.00|40|3
Orange|48.75|30|1
---
```

### **Sample Receipt File**
```
Receipts/Receipt_2024-08-14_10-30-15_John_Doe.txt
```
```
                    SUPERMARKET RECEIPT
============================================================
Customer: John Doe
Date: 2024-08-14 10:30:15

+------------------+-------+-----+----------+
| Item             | Rate  | Qty | Amount   |
+------------------+-------+-----+----------+
| Apple            | 55.50 |   2 |   111.00 |
| Milk 1L          | 65.00 |   1 |    65.00 |
| Bread            | 28.50 |   3 |    85.50 |
+------------------+-------+-----+----------+
                                 Subtotal:   261.50 BDT
                                 VAT(5%):     13.08 BDT
                                 Discount:     0.00 BDT
------------------------------------------------------------
                                NET TOTAL:   274.58 BDT
============================================================
```

### **Sample Daily Report**
```
Reports/DailyReport_2024-08-14.txt
```
```
======================================================================
             SUPERMARKET DAILY SALES REPORT
                    Date: 2024-08-14
======================================================================

SALES SUMMARY:
------------------------------
Total Transactions: 15
Total Revenue: 2847.50 BDT
Average Transaction: 189.83 BDT

TOP SELLING ITEMS:
--------------------------------------------------
Item                     Qty Sold    Visual Chart
--------------------------------------------------
Apple                    45           ████████████████████████████████████████████████
Milk 1L                  32           ████████████████████████████████████
Bread                    28           ████████████████████████████████
Banana                   25           █████████████████████████████
...
```

---

## 🚀 **Setup Instructions**

### **Method 1: Using Setup Script (Recommended)**
1. Download/create all files in a single folder
2. Run `setup_demo.bat` (Windows)
3. Follow the prompts
4. Program ready to use!

### **Method 2: Manual Setup**
```bash
# 1. Create project directory
mkdir BillMaster++
cd BillMaster++

# 2. Compile the source code
g++ -o BillMaster.exe BillMaster.cpp -std=c++17

# 3. Create required directories
mkdir Receipts Reports Backup

# 4. Run the program
./BillMaster.exe
```

---

## 📊 **File Size Estimates**

| Component | Estimated Size | Notes |
|-----------|----------------|--------|
| Source Code | ~15-20 KB | Single C++ file |
| Executable | ~500 KB - 2 MB | Depends on compiler/linking |
| Sample Data | ~1-2 KB | Initial inventory |
| Daily Reports | ~5-10 KB each | Depends on transactions |
| Receipts | ~1-3 KB each | Individual customer receipts |

---

## 🔧 **Maintenance Files**

### **.gitignore (Optional)**
```
# Executables
*.exe
*.out
BillMaster
pos_system

# Data files (keep local)
Sales.txt
Bill.txt

# Generated folders
Receipts/
Reports/

# IDE files
*.cbp
*.layout
.vscode/
```

### **LICENSE.txt (Optional)**
```
MIT License

Copyright (c) 2024 [Your Name]

Permission is hereby granted, free of charge, to any person obtaining a copy...
```

This file structure provides a complete, organized, and scalable foundation for your POS system that can handle both development and production use cases.
