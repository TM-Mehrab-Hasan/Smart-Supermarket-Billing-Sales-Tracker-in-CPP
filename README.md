# BillMaster++ 🛒

**BillMaster++** is a professional-grade **Enhanced Supermarket POS and Billing System** built in C++.  
It features advanced inventory management, barcode scanning, customer loyalty programs, multi-payment processing, comprehensive reporting, and modern UI — designed for real-world retail operations.

**Enterprise-Level Supermarket POS & Billing System in C++**  

![C++](https://img.shields.io/badge/Language-C++-blue)
![Cross-Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-green)
![License](https://img.shields.io/badge/License-MIT-yellow)
![Build](https://img.shields.io/badge/Build-Passing-brightgreen)
![Version](https://img.shields.io/badge/Version-2.0%20Enhanced-orange)

---

## ✨ Enhanced Features

### 🏪 **Core POS Functionality**
- **Advanced Inventory Management** with barcode support (8-13 digits)
- **Customer Loyalty Program** with points and purchase history
- **Multi-Payment Processing** (Cash, Card, Mobile Banking, Digital Wallet)
- **Professional Receipt Generation** with payment method details
- **Real-time Stock Management** with low stock alerts

### 📊 **Business Intelligence & Reporting**
- **Daily Business Reports** with comprehensive analytics
- **Sales History Analysis** with date filtering and summaries
- **Purchase Order Generation** for automated restocking
- **CSV Export** for external analysis
- **Dashboard Overview** with real-time statistics

### 🔍 **Advanced Search & Management**
- **Barcode Scanning Mode** for quick item lookup
- **Multi-criteria Search** (Name, Category, Barcode)
- **Category & Supplier Management**
- **Quick Sale Mode** for fast transactions
- **Bulk Operations** for inventory updates

### 🎨 **Modern User Interface**
- **Unicode Table Borders** for professional appearance
- **Color-coded Status Indicators** (OK/LOW/OUT stock)
- **Progress Bars** for operations
- **Cross-platform Compatibility** (Windows/Linux)
- **Enhanced Error Handling** with user-friendly messages

---

## 🏗️ **Project Structure**
```
BillMaster++/
│
├── 📄 Source Code
│   ├── BillMaster.cpp              # Main enhanced C++ source (2000+ lines)
│   └── README.md                   # This documentation
│
├── 🔧 Executables (Generated after compilation)
│   ├── BillMaster.exe              # Windows executable
│   └── BillMaster                  # Linux/Mac executable
│
├── 💾 Core Data Files (Auto-generated)
│   ├── Bill.txt                    # Enhanced inventory database
│   ├── Sales.txt                   # Sales transaction history
│   └── customers.txt               # Customer database with loyalty points
│
├── 📂 Generated Directories
│   ├── Receipts/                   # Customer receipts with payment details
│   │   ├── Receipt_2024-08-17_10-30-15_John_Doe.txt
│   │   └── Receipt_2024-08-17_14-22-33_Sarah_Smith.txt
│   │
│   ├── Reports/                    # Business reports and analytics
│   │   ├── DailyReport_2024-08-17.txt
│   │   ├── InventoryExport_2024-08-17.csv
│   │   └── PurchaseOrder_2024-08-17.txt
│   │
│   └── Backup/ (Optional)          # Automated backups
│       └── [Backup files with timestamps]
```

---

## 📄 **Enhanced File Formats**

### **Bill.txt (Enhanced Inventory Database)**
```
# Enhanced Inventory File - Format: Name|Rate|Quantity|Barcode|Category|Supplier|LowStockThreshold
Rice (Basmati 1kg)|85.50|150|8901030891234|Grains|ABC Foods Ltd|10
Milk (1 Liter)|65.00|80|8901526201014|Dairy|Mother Dairy|15
Coca Cola (500ml)|35.00|120|8901030891890|Beverages|Coca Cola Co|20
Chicken (1kg)|280.00|30|8901030893456|Meat|Premium Meat|3
Soap (Lux)|35.00|100|8901030897890|Personal Care|Unilever|15
```

### **Sales.txt (Transaction History)**
```
# Sales History - Format: DateTime|Customer|Amount|Items
2024-08-17 10:30:15|John Doe|275.50|Rice(1),Milk(2),Bread(1)
2024-08-17 14:22:33|Sarah Smith|185.75|Coca Cola(3),Chips(2),Chocolate(1)
2024-08-17 16:45:12|Quick Sale Customer|65.00|Milk(1)
```

### **customers.txt (Customer Database)**
```
# Customer Database - Format: ID|Name|Phone|Email|LoyaltyPoints|TotalSpent|VisitCount|LastVisit
CUST001|John Doe|01712345678|john@email.com|125.50|1250.75|8|1692259815
CUST002|Sarah Smith|01798765432|sarah@email.com|89.25|892.50|5|1692345612
```

### **Enhanced Receipt Sample**
```
================================================================
                    SUPERMARKET RECEIPT
================================================================
Customer: John Doe (Phone: 01712345678)
Loyalty Points: 125 (Earned: 12 points this purchase)
Date: 2024-08-17 10:30:15                    Bill #: 20240817001

┌────────────────────┬──────────┬─────┬─────────────┬──────────┐
│ Item               │ Rate     │ Qty │ Barcode     │ Amount   │
├────────────────────┼──────────┼─────┼─────────────┼──────────┤
│ Rice (Basmati 1kg) │    85.50 │   1 │ 8901030891234│    85.50 │
│ Milk (1 Liter)     │    65.00 │   2 │ 8901526201014│   130.00 │
│ Bread (White)      │    45.00 │   1 │ 8901030891567│    45.00 │
└────────────────────┴──────────┴─────┴─────────────┴──────────┘

                                            Subtotal:   260.50 BDT
                                            VAT (5%):    13.03 BDT
                                            Loyalty Discount: -10.00 BDT
                                            ─────────────────────────
                                            NET TOTAL:   263.53 BDT

PAYMENT DETAILS:
├─ Cash:                                               200.00 BDT
├─ Card (Visa ****1234):                                63.53 BDT
└─ Total Paid:                                         263.53 BDT

================================================================
        Thank you for shopping with us! Visit again soon!
             Next purchase: 15% off on items over 500 BDT
================================================================
```

### **Daily Report Sample**
```
===============================================
           DAILY BUSINESS REPORT
===============================================
Date: 2024-08-17 18:30:25
Report Generated: 2024-08-17 18:30:25

INVENTORY SUMMARY:
==================
Total Items: 48
Total Inventory Value: 45,750.50 BDT
Low Stock Items: 5
Out of Stock Items: 2

CUSTOMER SUMMARY:
=================
Total Customers: 127
Total Customer Spending: 127,450.75 BDT
Active Customers (Last 30 days): 89

TOP 5 CUSTOMERS BY SPENDING:
============================
1. John Doe - 1250.75 BDT (8 visits)
2. Sarah Smith - 892.50 BDT (5 visits)
3. Mike Johnson - 745.25 BDT (6 visits)
4. Emma Wilson - 650.00 BDT (4 visits)
5. David Brown - 580.75 BDT (7 visits)

CATEGORY ANALYSIS:
==================
Dairy: 8 items, Value: 5,240.00 BDT
Beverages: 12 items, Value: 8,750.50 BDT
Grains: 6 items, Value: 4,200.00 BDT
Personal Care: 10 items, Value: 3,850.75 BDT

RECOMMENDATIONS:
================
- Restock 5 low stock items
- Urgently restock 2 out of stock items
===============================================
```

---

## 🚀 **Setup & Installation**

### **Prerequisites**
- **C++ Compiler** (GCC 7.0+ or Visual Studio 2017+)
- **C++17 Standard** support
- **Windows 10+** or **Linux** (Ubuntu 18.04+)

### **Quick Setup**
```bash
# 1. Clone the repository
git clone https://github.com/TM-Mehrab-Hasan/Smart-Supermarket-Billing-Sales-Tracker-in-CPP.git
cd BillMaster++

# 2. Compile the source code
# For Windows:
g++ -o BillMaster.exe BillMaster.cpp -static -std=c++17

# For Linux/Mac:
g++ -o BillMaster BillMaster.cpp -std=c++17

# 3. Run the program
# Windows:
./BillMaster.exe

# Linux/Mac:
./BillMaster
```

### **Features Demo Setup**
```bash
# Create sample data directories
mkdir Receipts Reports Backup

# The program will auto-generate sample data on first run
# Or manually add sample inventory items to Bill.txt
```

---

## 🎮 **Usage Guide**

### **Main Menu Options**
```
╔══════════════════════════════════════════════════════════════════╗
║                    🏪 BILLMASTER++ POS SYSTEM                    ║
║                        Enhanced Edition v2.0                     ║
╠══════════════════════════════════════════════════════════════════╣
║  1. 📦 Inventory Management     │  7. 📊 View Sales History      ║
║  2. 💰 Customer Billing         │  8. 👥 Customer Management     ║
║  3. 🔍 Search Inventory         │  9. 📱 Barcode Scan Mode       ║
║  4. 📋 Display Inventory        │ 10. 📈 Dashboard Overview       ║
║  5. ⚡ Quick Sale              │ 11. 📄 Generate Daily Report   ║
║  6. 🛒 Generate Purchase Order  │ 12. 💾 Export to CSV           ║
║                                 │  0. 🚪 Exit System             ║
╚══════════════════════════════════════════════════════════════════╝
```

### **Key Workflows**

#### **Adding New Inventory**
1. Select **"1. Inventory Management"** → **"1. Add New Item"**
2. Enter item details with barcode (8-13 digits)
3. Set category, supplier, and low stock threshold
4. System validates and saves to database

#### **Customer Billing Process**
1. Select **"2. Customer Billing"**
2. Choose customer lookup method or create new
3. Scan/search items by name or barcode
4. Select payment method(s) - supports split payments
5. Generate professional receipt with loyalty points

#### **Quick Sale (Fast Transactions)**
1. Select **"5. Quick Sale"**
2. Enter item name or scan barcode
3. Specify quantity and confirm
4. Process payment and generate receipt
5. Automatically records in sales history

---

## 🔧 **Advanced Features**

### **Barcode Support**
- **Validation**: 8-13 digit numeric codes
- **Scanning Mode**: Dedicated barcode lookup interface
- **Integration**: Works in billing, search, and quick sale

### **Customer Loyalty Program**
- **Points System**: 1 point per 10 BDT spent
- **Discounts**: Automatic loyalty discounts
- **History Tracking**: Complete purchase history per customer

### **Multi-Payment Processing**
- **Payment Types**: Cash, Card, Mobile Banking, Digital
