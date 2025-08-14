#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <algorithm>
#include <limits>

#ifdef _WIN32
    #include <windows.h>
    #include <filesystem>
    namespace fs = std::filesystem;
#else
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
    #include <unistd.h>
    #include <sys/stat.h>
#endif

using namespace std;

struct ItemRec { 
    string name; 
    double rate = 0.0;  // Changed to double for better precision
    int qty = 0; 
};

// Configuration constants
const string INVENTORY_FILE = "Bill.txt";
const string SALES_FILE = "Sales.txt";
const string RECEIPT_FOLDER = "Receipts";
const string REPORT_FOLDER = "Reports";
const int LOW_STOCK_THRESHOLD = 5;  // Configurable low stock threshold
const double VAT_RATE = 0.05;       // 5% VAT
const double DISCOUNT_THRESHOLD = 500.0;
const double DISCOUNT_AMOUNT = 50.0;

// Function declarations (forward declarations)
void display_inventory();
void search_inventory();
void delete_item_flow();
void generate_daily_report();
void view_sales_history();
void add_item_flow();
void print_bill_flow();

// Cross-platform console color support
void setColor(int color) {
    #ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    #else
        // Basic color support for Unix-like systems
        switch(color) {
            case 4: cout << "\033[31m"; break;  // Red
            case 7: cout << "\033[37m"; break;  // White
            case 9: cout << "\033[94m"; break;  // Light Blue
            case 10: cout << "\033[32m"; break; // Green
            case 11: cout << "\033[36m"; break; // Cyan
            case 13: cout << "\033[95m"; break; // Magenta
            case 14: cout << "\033[33m"; break; // Yellow
            default: cout << "\033[0m"; break;  // Reset
        }
    #endif
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pauseSystem() {
    #ifdef _WIN32
        system("pause");
    #else
        cout << "Press Enter to continue...";
        cin.get();
    #endif
}

void sleepMs(int ms) {
    #ifdef _WIN32
        Sleep(ms);
    #else
        usleep(ms * 1000);
    #endif
}

void wait_and_flush() { 
    cin.clear(); 
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
}

// Enhanced parsing with better error handling
bool parse_line(const string& line, ItemRec& out) {
    if(line.empty() || line[0] == '#') return false; // Skip empty lines and comments
    
    size_t p1 = line.find('|');
    size_t p2 = line.find('|', p1 + 1);
    if(p1 == string::npos || p2 == string::npos) return false;
    
    out.name = line.substr(0, p1);
    // Trim whitespace from name
    out.name.erase(0, out.name.find_first_not_of(" \t"));
    out.name.erase(out.name.find_last_not_of(" \t") + 1);
    
    try { 
        out.rate = stod(line.substr(p1+1, p2-p1-1)); 
        out.qty = stoi(line.substr(p2+1)); 
    } catch (const exception& e) { 
        return false; 
    }
    
    return !out.name.empty() && out.rate >= 0 && out.qty >= 0;
}

string serialize_line(const ItemRec& r) { 
    return r.name + "|" + to_string(r.rate) + "|" + to_string(r.qty); 
}

vector<ItemRec> load_inventory() {
    vector<ItemRec> inv; 
    ifstream in(INVENTORY_FILE);
    if(!in.is_open()) {
        // Create empty inventory file if it doesn't exist
        ofstream out(INVENTORY_FILE);
        out.close();
        return inv;
    }
    
    string line; 
    while(getline(in, line)) { 
        ItemRec r; 
        if(parse_line(line, r)) {
            inv.push_back(r); 
        }
    }
    in.close();
    return inv;
}

bool save_inventory(const vector<ItemRec>& inv) {
    ofstream out(INVENTORY_FILE, ios::trunc);
    if(!out.is_open()) return false; 
    
    out << "# Inventory File - Format: Name|Rate|Quantity\n";
    for(const auto &r : inv) {
        out << serialize_line(r) << "\n"; 
    }
    out.close();
    return true;
}

int find_item(const vector<ItemRec>& inv, const string &name) {
    for(size_t i = 0; i < inv.size(); i++) {
        if(inv[i].name == name) return static_cast<int>(i); 
    }
    return -1;
}

// Enhanced receipt printing with better formatting
void print_receipt(const vector<pair<ItemRec,int>>& items, double total, double vat, double discount, double net_total, const string& customer = "") {
    setColor(11);
    cout << "\n" << string(60, '=') << "\n";
    cout << "                    SUPERMARKET RECEIPT\n";
    cout << string(60, '=') << "\n";
    setColor(7);
    
    if(!customer.empty()) {
        cout << "Customer: " << customer << "\n";
    }
    
    time_t t = time(0);
    tm* now = localtime(&t);
    char buf[30];
    strftime(buf, 30, "%Y-%m-%d %H:%M:%S", now);
    cout << "Date: " << buf << "\n\n";
    
    setColor(11);
    cout << "+------------------+-------+-----+----------+\n";
    cout << "| Item             | Rate  | Qty | Amount   |\n";
    cout << "+------------------+-------+-----+----------+\n"; 
    setColor(7);
    
    for(const auto &p : items) {
        const auto &it = p.first; 
        int q = p.second;
        double amount = it.rate * q;
        
        cout << "| " << left << setw(16) << it.name.substr(0, 16)
             << " | " << right << setw(5) << fixed << setprecision(2) << it.rate
             << " | " << setw(3) << q
             << " | " << setw(8) << amount << " |\n";
    }
    
    setColor(11);
    cout << "+------------------+-------+-----+----------+\n"; 
    setColor(14);
    cout << fixed << setprecision(2);
    cout << right << setw(45) << "Subtotal: " << setw(8) << total << " BDT\n";
    cout << right << setw(45) << "VAT(" << (VAT_RATE*100) << "%): " << setw(8) << vat << " BDT\n";
    if(discount > 0) {
        cout << right << setw(45) << "Discount: " << setw(8) << discount << " BDT\n";
    }
    setColor(11);
    cout << string(60, '-') << "\n";
    setColor(10);
    cout << right << setw(45) << "NET TOTAL: " << setw(8) << net_total << " BDT\n";
    setColor(11);
    cout << string(60, '=') << "\n";
    setColor(7);
}

// Check for low stock items
void check_low_stock(const vector<ItemRec>& inv) {
    vector<ItemRec> low_stock_items;
    for(const auto& item : inv) {
        if(item.qty <= LOW_STOCK_THRESHOLD && item.qty > 0) {
            low_stock_items.push_back(item);
        }
    }
    
    if(!low_stock_items.empty()) {
        setColor(4);
        cout << "\n⚠️ LOW STOCK ALERT! ⚠️\n";
        setColor(14);
        cout << "The following items are running low:\n";
        setColor(7);
        for(const auto& item : low_stock_items) {
            cout << "- " << item.name << " (Only " << item.qty << " left)\n";
        }
        cout << "\n";
        sleepMs(3000);
    }
}

// Create necessary directories
void ensure_directories() {
    try {
        if(!fs::exists(RECEIPT_FOLDER)) {
            fs::create_directory(RECEIPT_FOLDER);
        }
        if(!fs::exists(REPORT_FOLDER)) {
            fs::create_directory(REPORT_FOLDER);
        }
    } catch(const exception& e) {
        setColor(4);
        cout << "Error creating directories: " << e.what() << "\n";
        setColor(7);
    }
}

// ----------------- Display / Search Inventory -----------------
void display_inventory() { 
    auto inv = load_inventory();
    if(inv.empty()) { 
        setColor(4); 
        cout << "Inventory is empty!\n"; 
        setColor(7); 
        sleepMs(2000); 
        return; 
    }
    
    clearScreen(); 
    setColor(11);
    cout << "\n\t=== CURRENT INVENTORY ===\n\n";
    cout << "+------------------+----------+-------+--------+\n";
    cout << "| Item             | Rate     | Qty   | Status |\n";
    cout << "+------------------+----------+-------+--------+\n"; 
    setColor(7);
    
    for(const auto &it : inv) {
        cout << "| " << left << setw(16) << it.name.substr(0, 16)
             << " | " << right << setw(8) << fixed << setprecision(2) << it.rate
             << " | " << setw(5) << it.qty << " | ";
        
        if(it.qty == 0) {
            setColor(4); cout << " OUT   ";
        } else if(it.qty <= LOW_STOCK_THRESHOLD) {
            setColor(14); cout << " LOW   ";
        } else {
            setColor(10); cout << " OK    ";
        }
        setColor(7);
        cout << " |\n";
    }
    
    setColor(11); 
    cout << "+------------------+----------+-------+--------+\n"; 
    cout << "\nTotal Items: " << inv.size() << "\n";
    setColor(7); 
    pauseSystem();
}

void search_inventory() { 
    auto inv = load_inventory();
    if(inv.empty()) { 
        setColor(4); 
        cout << "Inventory is empty!\n"; 
        setColor(7); 
        sleepMs(2000); 
        return; 
    }
    
    setColor(14); 
    cout << "Enter search term: "; 
    setColor(7); 
    string term; 
    getline(cin, term);
    
    if(term.empty()) {
        setColor(4);
        cout << "Search term cannot be empty!\n";
        setColor(7);
        sleepMs(1500);
        return;
    }
    
    // Convert search term to lowercase for case-insensitive search
    transform(term.begin(), term.end(), term.begin(), ::tolower);
    
    vector<ItemRec> results; 
    for(const auto &it : inv) {
        string item_name = it.name;
        transform(item_name.begin(), item_name.end(), item_name.begin(), ::tolower);
        if(item_name.find(term) != string::npos) {
            results.push_back(it);
        }
    }
    
    if(results.empty()) { 
        setColor(4); 
        cout << "No items found matching '" << term << "'\n"; 
        setColor(7); 
        sleepMs(2000); 
        return; 
    }
    
    clearScreen(); 
    setColor(11); 
    cout << "\n\t=== SEARCH RESULTS for '" << term << "' ===\n\n";
    cout << "+------------------+----------+-------+--------+\n";
    cout << "| Item             | Rate     | Qty   | Status |\n";
    cout << "+------------------+----------+-------+--------+\n"; 
    setColor(7);
    
    for(const auto &it : results) {
        cout << "| " << left << setw(16) << it.name.substr(0, 16)
             << " | " << right << setw(8) << fixed << setprecision(2) << it.rate
             << " | " << setw(5) << it.qty << " | ";
        
        if(it.qty == 0) {
            setColor(4); cout << " OUT   ";
        } else if(it.qty <= LOW_STOCK_THRESHOLD) {
            setColor(14); cout << " LOW   ";
        } else {
            setColor(10); cout << " OK    ";
        }
        setColor(7);
        cout << " |\n";
    }
    
    setColor(11); 
    cout << "+------------------+----------+-------+--------+\n"; 
    cout << "\nFound " << results.size() << " matching items.\n";
    setColor(7); 
    pauseSystem();
}

// ----------------- Add / Update Item -----------------
void add_item_flow() {
    bool close = false;
    while(!close) {
        clearScreen(); 
        setColor(11); 
        cout << "\n\t=== INVENTORY MANAGEMENT ===\n";
        cout << "\t1. Add New Item\n\t2. Update Existing Item\n\t3. View Current Stock\n\t4. Back to Main Menu\n";
        cout << "\tEnter Choice: "; 
        setColor(7);
        
        int choice; 
        if(!(cin >> choice)) { 
            wait_and_flush(); 
            continue; 
        } 
        wait_and_flush();
        
        if(choice == 1 || choice == 2) {
            string name; 
            double rate;
            int quant;
            
            setColor(14); 
            cout << "\tEnter Item Name: "; 
            setColor(7); 
            getline(cin, name);
            
            if(name.empty()) {
                setColor(4); 
                cout << "Item name cannot be empty!\n"; 
                setColor(7); 
                sleepMs(1500); 
                continue;
            }
            
            setColor(14); 
            cout << "\tEnter Rate (BDT): "; 
            setColor(7); 
            if(!(cin >> rate) || rate < 0) {
                setColor(4); 
                cout << "Invalid rate! Please enter a positive number.\n"; 
                setColor(7); 
                wait_and_flush();
                sleepMs(1500); 
                continue;
            }
            
            setColor(14); 
            cout << "\tEnter Quantity: "; 
            setColor(7); 
            if(!(cin >> quant) || quant < 0) {
                setColor(4); 
                cout << "Invalid quantity! Please enter a positive number.\n"; 
                setColor(7); 
                wait_and_flush();
                sleepMs(1500); 
                continue;
            }
            wait_and_flush();
            
            auto inv = load_inventory(); 
            int idx = find_item(inv, name);
            
            if(idx >= 0) {
                setColor(14);
                cout << "Item exists! Current: Rate=" << inv[idx].rate << ", Qty=" << inv[idx].qty << "\n";
                if(choice == 1) {
                    cout << "Adding to existing quantity...\n";
                    inv[idx].rate = rate; 
                    inv[idx].qty += quant;
                } else {
                    cout << "Updating item...\n";
                    inv[idx].rate = rate; 
                    inv[idx].qty = quant;  // Replace quantity instead of adding
                }
                setColor(7);
            } else {
                inv.push_back({name, rate, quant});
            }
            
            if(!save_inventory(inv)) {
                setColor(4);
                cout << "Error: Cannot save inventory!\n";
                setColor(7);
            } else {
                setColor(10);
                cout << "✓ Item successfully " << (idx >= 0 ? "updated" : "added") << "!\n";
                setColor(7);
            }
            sleepMs(2000);
            
        } else if(choice == 3) {
            display_inventory();
        } else if(choice == 4) {
            close = true;
        } else {
            setColor(4);
            cout << "Invalid choice! Please try again.\n";
            setColor(7);
            sleepMs(1000);
        }
    }
}

// ----------------- Print Bill / Sell Items -----------------
void print_bill_flow() {
    clearScreen(); 
    auto inv = load_inventory();
    if(inv.empty()) { 
        setColor(4); 
        cout << "Inventory is empty! Please add items first.\n"; 
        setColor(7); 
        sleepMs(2000); 
        return; 
    }
    
    string customer; 
    setColor(14); 
    cout << "Enter customer name (or press Enter for walk-in): "; 
    setColor(7); 
    getline(cin, customer);
    if(customer.empty()) customer = "Walk-in Customer";

    vector<pair<ItemRec,int>> sold_items; 
    double total = 0; 
    bool close = false;
    
    while(!close) {
        clearScreen(); 
        setColor(11);
        cout << "\n\t=== BILLING SYSTEM ===\n";
        cout << "\tCustomer: " << customer << "\n";
        cout << "\tCurrent Bill Total: " << fixed << setprecision(2) << total << " BDT\n\n";
        cout << "\t1. Add Item to Bill\n\t2. View Current Bill\n\t3. Complete Sale\n\t4. Cancel Sale\n";
        cout << "\tEnter Choice: "; 
        setColor(7);
        
        int choice; 
        if(!(cin >> choice)) { 
            wait_and_flush(); 
            continue; 
        } 
        wait_and_flush();
        
        if(choice == 1) {
            string name; 
            int q; 
            setColor(14); 
            cout << "Item name: "; 
            setColor(7); 
            getline(cin, name);
            
            if(name.empty()) {
                setColor(4); 
                cout << "Item name cannot be empty!\n"; 
                setColor(7); 
                sleepMs(1500); 
                continue;
            }
            
            setColor(14); 
            cout << "Quantity: "; 
            setColor(7); 
            if(!(cin >> q) || q <= 0) { 
                setColor(4); 
                cout << "Invalid quantity! Must be positive.\n"; 
                setColor(7); 
                wait_and_flush(); 
                sleepMs(1500); 
                continue; 
            }
            wait_and_flush();
            
            int idx = find_item(inv, name); 
            if(idx < 0) { 
                setColor(4); 
                cout << "Item '" << name << "' not found in inventory!\n"; 
                setColor(7); 
                sleepMs(2000); 
                continue; 
            }
            
            if(inv[idx].qty < q) { 
                setColor(4); 
                cout << "Insufficient stock! Only " << inv[idx].qty << " items available.\n"; 
                setColor(7); 
                sleepMs(2000); 
                continue; 
            }
            
            // Update inventory and add to bill
            inv[idx].qty -= q; 
            sold_items.push_back({inv[idx], q}); 
            total += inv[idx].rate * q;
            save_inventory(inv);
            
            setColor(10);
            cout << "✓ Added " << q << " x " << name << " to bill (₹" << (inv[idx].rate * q) << ")\n";
            setColor(7);
            sleepMs(1500);
            
        } else if(choice == 2) {
            if(sold_items.empty()) {
                setColor(14);
                cout << "Bill is currently empty.\n";
                setColor(7);
                sleepMs(1500);
                continue;
            }
            
            clearScreen();
            double vat = total * VAT_RATE;
            double discount = total > DISCOUNT_THRESHOLD ? DISCOUNT_AMOUNT : 0;
            double net_total = total + vat - discount;
            print_receipt(sold_items, total, vat, discount, net_total, customer);
            pauseSystem();
            
        } else if(choice == 3) {
            if(sold_items.empty()) {
                setColor(4);
                cout << "Cannot complete sale - no items in bill!\n";
                setColor(7);
                sleepMs(1500);
                continue;
            }
            close = true;
        } else if(choice == 4) {
            // Cancel sale - restore inventory
            for(const auto& item : sold_items) {
                int idx = find_item(inv, item.first.name);
                if(idx >= 0) {
                    inv[idx].qty += item.second;
                }
            }
            save_inventory(inv);
            setColor(14);
            cout << "Sale cancelled. Inventory restored.\n";
            setColor(7);
            sleepMs(1500);
            return;
        } else {
            setColor(4); 
            cout << "Invalid choice!\n"; 
            setColor(7); 
            sleepMs(1000);
        }
    }

    // Process final sale
    double vat = total * VAT_RATE; 
    double discount = total > DISCOUNT_THRESHOLD ? DISCOUNT_AMOUNT : 0; 
    double net_total = total + vat - discount;
    
    clearScreen(); 
    setColor(13); 
    cout << "\n\t==== FINAL RECEIPT ====\n"; 
    setColor(7);
    print_receipt(sold_items, total, vat, discount, net_total, customer);

    // Save sale to file
    ensure_directories();
    ofstream out(SALES_FILE, ios::app);
    time_t t = time(0); 
    tm* now = localtime(&t);
    char buf[30]; 
    strftime(buf, 30, "%Y-%m-%d %H:%M:%S", now);
    
    out << buf << "|" << customer << "|" << fixed << setprecision(2) << net_total << "\n";
    for(const auto &p : sold_items) {
        out << serialize_line(p.first) << "|" << p.second << "\n"; 
    }
    out << "---\n"; 
    out.close();

    // Save receipt file
    string safe_customer = customer;
    replace(safe_customer.begin(), safe_customer.end(), ' ', '_');
    replace(safe_customer.begin(), safe_customer.end(), '/', '_');
    
    strftime(buf, 30, "%Y-%m-%d_%H-%M-%S", now);
    string filename = RECEIPT_FOLDER + "/Receipt_" + buf + "_" + safe_customer + ".txt";
    
    ofstream rec(filename); 
    rec << "                    SUPERMARKET RECEIPT\n";
    rec << string(60, '=') << "\n";
    rec << "Customer: " << customer << "\n";
    rec << "Date: " << buf << "\n\n";
    rec << "+------------------+-------+-----+----------+\n";
    rec << "| Item             | Rate  | Qty | Amount   |\n";
    rec << "+------------------+-------+-----+----------+\n";
    
    for(const auto &p : sold_items) {
        rec << "| " << left << setw(16) << p.first.name.substr(0, 16)
            << " | " << right << setw(5) << fixed << setprecision(2) << p.first.rate
            << " | " << setw(3) << p.second
            << " | " << setw(8) << (p.first.rate * p.second) << " |\n";
    }
    
    rec << "+------------------+-------+-----+----------+\n";
    rec << fixed << setprecision(2);
    rec << right << setw(45) << "Subtotal: " << setw(8) << total << " BDT\n";
    rec << right << setw(45) << "VAT(" << (VAT_RATE*100) << "%): " << setw(8) << vat << " BDT\n";
    if(discount > 0) {
        rec << right << setw(45) << "Discount: " << setw(8) << discount << " BDT\n";
    }
    rec << string(60, '-') << "\n";
    rec << right << setw(45) << "NET TOTAL: " << setw(8) << net_total << " BDT\n";
    rec << string(60, '=') << "\n";
    rec.close();

    setColor(10); 
    cout << "\n✓ Sale completed successfully!\n"; 
    cout << "✓ Receipt saved to: " << filename << "\n";
    setColor(7); 
    
    // Check for low stock after sale
    check_low_stock(inv);
    
    sleepMs(3000);
}

// ----------------- Enhanced Daily Report -----------------
void generate_daily_report() {
    ensure_directories();
    
    time_t t = time(0); 
    tm* now = localtime(&t); 
    char date_buf[20]; 
    strftime(date_buf, 20, "%Y-%m-%d", now);
    
    string report_file = REPORT_FOLDER + "/DailyReport_" + date_buf + ".txt";
    ifstream in(SALES_FILE); 
    
    if(!in.is_open()) { 
        setColor(4);
        cout << "No sales data found! Make some sales first.\n"; 
        setColor(7);
        sleepMs(2000);
        return; 
    }

    map<string, int> item_sales;
    map<string, double> customer_sales; 
    double total_sales = 0;
    int total_transactions = 0;
    string line;

    // Parse sales data
    string current_customer; 
    while(getline(in, line)) {
        if(line == "---") { 
            current_customer = ""; 
            total_transactions++;
            continue; 
        }
        
        if(line.find('|') != string::npos && current_customer.empty()) {
            // This is a transaction header
            stringstream ss(line); 
            string datetime, customer, net_str;
            getline(ss, datetime, '|'); 
            getline(ss, customer, '|'); 
            getline(ss, net_str);
            
            if(!net_str.empty()) {
                current_customer = customer;
                double net_amount = stod(net_str);
                customer_sales[customer] += net_amount;
                total_sales += net_amount;
            }
        } else if(!line.empty() && !current_customer.empty()) {
            // This is an item line
            stringstream ss(line); 
            string name, rate_str, qty_str, sold_qty_str;
            getline(ss, name, '|'); 
            getline(ss, rate_str, '|'); 
            getline(ss, qty_str, '|');
            getline(ss, sold_qty_str);
            
            if(!sold_qty_str.empty()) {
                int qty = stoi(sold_qty_str);
                item_sales[name] += qty;
            }
        }
    }
    in.close();

    // Generate report
    ofstream out(report_file, ios::trunc);
    out << string(70, '=') << "\n";
    out << "             SUPERMARKET DAILY SALES REPORT\n";
    out << "                    Date: " << date_buf << "\n";
    out << string(70, '=') << "\n\n";

    // Sales Summary
    out << "SALES SUMMARY:\n";
    out << string(30, '-') << "\n";
    out << "Total Transactions: " << total_transactions << "\n";
    out << "Total Revenue: " << fixed << setprecision(2) << total_sales << " BDT\n";
    out << "Average Transaction: " << (total_transactions > 0 ? total_sales/total_transactions : 0) << " BDT\n\n";

    // Top selling items
    if(!item_sales.empty()) {
        out << "TOP SELLING ITEMS:\n";
        out << string(50, '-') << "\n";
        out << left << setw(25) << "Item" << setw(12) << "Qty Sold" << "Visual Chart\n";
        out << string(50, '-') << "\n";
        
        // Sort items by quantity sold (descending)
        vector<pair<string, int>> sorted_items(item_sales.begin(), item_sales.end());
        sort(sorted_items.begin(), sorted_items.end(), 
             [](const pair<string, int>& a, const pair<string, int>& b) {
                 return a.second > b.second;
             });
        
        for(const auto &p : sorted_items) {
            out << left << setw(25) << p.first.substr(0, 24) 
                << setw(12) << p.second << " ";
            // Create simple bar chart
            int bars = min(30, p.second);  // Max 30 characters for chart
            for(int i = 0; i < bars; i++) out << "█";
            out << "\n";
        }
        out << "\n";
    }

    // Customer sales summary
    if(!customer_sales.empty()) {
        out << "CUSTOMER SALES SUMMARY:\n";
        out << string(45, '-') << "\n";
        out << left << setw(25) << "Customer" << setw(15) << "Total Spent" << "\n";
        out << string(45, '-') << "\n";
        
        // Sort customers by amount spent (descending)
        vector<pair<string, double>> sorted_customers(customer_sales.begin(), customer_sales.end());
        sort(sorted_customers.begin(), sorted_customers.end(), 
             [](const pair<string, double>& a, const pair<string, double>& b) {
                 return a.second > b.second;
             });
        
        for(const auto &p : sorted_customers) {
            out << left << setw(25) << p.first.substr(0, 24) 
                << setw(15) << fixed << setprecision(2) << p.second << " BDT\n";
        }
        out << "\n";
    }

    // Current inventory status
    auto inv = load_inventory();
    if(!inv.empty()) {
        out << "CURRENT INVENTORY STATUS:\n";
        out << string(60, '-') << "\n";
        out << left << setw(20) << "Item" << setw(10) << "Rate" << setw(8) << "Stock" << setw(10) << "Status\n";
        out << string(60, '-') << "\n";
        
        int out_of_stock = 0, low_stock = 0, normal_stock = 0;
        
        for(const auto &item : inv) {
            out << left << setw(20) << item.name.substr(0, 19) 
                << setw(10) << fixed << setprecision(2) << item.rate 
                << setw(8) << item.qty << setw(10);
            
            if(item.qty == 0) {
                out << "OUT OF STOCK";
                out_of_stock++;
            } else if(item.qty <= LOW_STOCK_THRESHOLD) {
                out << "LOW STOCK";
                low_stock++;
            } else {
                out << "OK";
                normal_stock++;
            }
            out << "\n";
        }
        
        out << string(60, '-') << "\n";
        out << "Inventory Summary:\n";
        out << "- Total Items: " << inv.size() << "\n";
        out << "- Normal Stock: " << normal_stock << "\n";
        out << "- Low Stock: " << low_stock << "\n";
        out << "- Out of Stock: " << out_of_stock << "\n\n";
    }

    out << string(70, '=') << "\n";
    out << "Report generated on: ";
    time_t report_time = time(0);
    tm* report_tm = localtime(&report_time);
    char time_buf[30];
    strftime(time_buf, 30, "%Y-%m-%d %H:%M:%S", report_tm);
    out << time_buf << "\n";
    out << string(70, '=') << "\n";
    out.close();

    clearScreen();
    setColor(10);
    cout << "\n✓ Daily report generated successfully!\n";
    cout << "✓ Report saved to: " << report_file << "\n\n";
    setColor(11);
    cout << "REPORT SUMMARY:\n";
    cout << string(40, '-') << "\n";
    setColor(7);
    cout << "Total Transactions: " << total_transactions << "\n";
    cout << "Total Revenue: " << fixed << setprecision(2) << total_sales << " BDT\n";
    cout << "Items Sold: " << item_sales.size() << " different items\n";
    cout << "Customers Served: " << customer_sales.size() << "\n";
    
    if(total_transactions > 0) {
        cout << "Average Transaction: " << (total_sales/total_transactions) << " BDT\n";
    }
    
    setColor(7);
    pauseSystem();
}

// ----------------- Delete Item Function -----------------
void delete_item_flow() {
    auto inv = load_inventory();
    if(inv.empty()) {
        setColor(4);
        cout << "Inventory is empty!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    setColor(14);
    cout << "Enter item name to delete: ";
    setColor(7);
    string name;
    getline(cin, name);
    
    if(name.empty()) {
        setColor(4);
        cout << "Item name cannot be empty!\n";
        setColor(7);
        sleepMs(1500);
        return;
    }
    
    int idx = find_item(inv, name);
    if(idx < 0) {
        setColor(4);
        cout << "Item '" << name << "' not found!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    setColor(14);
    cout << "Are you sure you want to delete '" << inv[idx].name << "'? (y/n): ";
    setColor(7);
    char confirm;
    cin >> confirm;
    wait_and_flush();
    
    if(confirm == 'y' || confirm == 'Y') {
        inv.erase(inv.begin() + idx);
        if(save_inventory(inv)) {
            setColor(10);
            cout << "✓ Item '" << name << "' deleted successfully!\n";
            setColor(7);
        } else {
            setColor(4);
            cout << "Error: Could not save inventory!\n";
            setColor(7);
        }
    } else {
        setColor(14);
        cout << "Delete operation cancelled.\n";
        setColor(7);
    }
    sleepMs(2000);
}

// ----------------- View Sales History -----------------
void view_sales_history() {
    ifstream in(SALES_FILE);
    if(!in.is_open()) {
        setColor(4);
        cout << "No sales history found!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    clearScreen();
    setColor(11);
    cout << "\n\t=== SALES HISTORY ===\n\n";
    setColor(7);
    
    string line;
    int transaction_count = 0;
    double total_revenue = 0;
    
    cout << left << setw(20) << "Date/Time" << setw(20) << "Customer" << setw(15) << "Amount" << "\n";
    cout << string(55, '-') << "\n";
    
    while(getline(in, line)) {
        if(line == "---") {
            transaction_count++;
            continue;
        }
        
        if(line.find('|') != string::npos && line.find(':') != string::npos) {
            stringstream ss(line);
            string datetime, customer, amount_str;
            getline(ss, datetime, '|');
            getline(ss, customer, '|');
            getline(ss, amount_str);
            
            if(!amount_str.empty()) {
                double amount = stod(amount_str);
                total_revenue += amount;
                
                cout << left << setw(20) << datetime.substr(0, 19)
                     << setw(20) << customer.substr(0, 19)
                     << setw(15) << fixed << setprecision(2) << amount << " BDT\n";
            }
        }
    }
    
    in.close();
    
    cout << string(55, '-') << "\n";
    setColor(10);
    cout << "Total Transactions: " << transaction_count << "\n";
    cout << "Total Revenue: " << fixed << setprecision(2) << total_revenue << " BDT\n";
    setColor(7);
    
    pauseSystem();
}

// ----------------- Enhanced Main Menu -----------------
int main() {
    #ifdef _WIN32
        // Set console to UTF-8 encoding
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif
    
    ensure_directories();
    
    bool exit = false;
    while(!exit) {
        clearScreen(); 
        
        // Check for low stock items on startup
        auto inv = load_inventory();
        check_low_stock(inv);
        
        setColor(9); 
        cout << "\n";
        cout << "\t██████╗ ██╗██╗     ██╗      ███╗   ███╗ █████╗ ███████╗████████╗███████╗██████╗  \n";
        cout << "\t██╔══██╗██║██║     ██║      ████╗ ████║██╔══██╗██╔════╝╚══██╔══╝██╔════╝██╔══██╗ \n";
        cout << "\t██████╔╝██║██║     ██║█████╗██╔████╔██║███████║███████╗   ██║   █████╗  ██████╔╝ \n";
        cout << "\t██╔══██╗██║██║     ██║╚════╝██║╚██╔╝██║██╔══██║╚════██║   ██║   ██╔══╝  ██╔══██╗ \n";
        cout << "\t██████╔╝██║███████╗███████  ██║ ╚═╝ ██║██║  ██║███████║   ██║   ███████╗██║  ██║ \n";
        cout << "\t╚═════╝ ╚═╝╚══════╝╚══════╝ ╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝ \n";
        setColor(7);
        
        cout << "\n\t\t\t    Supermarket POS & Billing System v2.0\n";
        cout << "\t\t\t    =====================================\n\n";
        
        setColor(11);
        cout << "\t┌────────────────────────────────────────────────────────────┐\n";
        cout << "\t│                        MAIN MENU                           │\n";
        cout << "\t├────────────────────────────────────────────────────────────┤\n";
        cout << "\t│  1. 📦 Inventory Management (Add/Update Items)             │\n";
        cout << "\t│  2. 💰 Create Bill / Process Sale                          │\n";
        cout << "\t│  3. 📋 Display Full Inventory                              │\n";
        cout << "\t│  4. 🔍 Search Inventory                                    │\n";
        cout << "\t│  5. 🗑️ Delete Item from Inventory                          │\n";
        cout << "\t│  6. 📊 Generate Daily Report                               │\n";
        cout << "\t│  7. 📈 View Sales History                                  │\n";
        cout << "\t│  8. ❌ Exit System                                         │\n";
        cout << "\t└────────────────────────────────────────────────────────────┘\n";
        setColor(7);
        
        cout << "\n\tEnter your choice (1-8): ";
        
        int val; 
        if(!(cin >> val)) { 
            wait_and_flush(); 
            setColor(4);
            cout << "\tInvalid input! Please enter a number.\n";
            setColor(7);
            sleepMs(1500);
            continue; 
        } 
        wait_and_flush();
        
        switch(val) {
            case 1: 
                add_item_flow(); 
                break;
            case 2: 
                print_bill_flow(); 
                break;
            case 3: 
                display_inventory(); 
                break;
            case 4: 
                search_inventory(); 
                break;
            case 5:
                delete_item_flow();
                break;
            case 6: 
                generate_daily_report(); 
                break;
            case 7:
                view_sales_history();
                break;
            case 8: 
                setColor(14);
                cout << "\n\tThank you for using POS System! Goodbye! 👋\n";
                setColor(7);
                sleepMs(2000);
                exit = true; 
                break;
            default: 
                setColor(4); 
                cout << "\tInvalid choice! Please select 1-8.\n"; 
                setColor(7); 
                sleepMs(1500);
        }
    }
    return 0;
}