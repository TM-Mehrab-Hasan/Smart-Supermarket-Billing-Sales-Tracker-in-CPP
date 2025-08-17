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
#include <regex>

// Remove filesystem dependencies for better portability
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <conio.h>
    #define MKDIR(dir) _mkdir(dir)
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <termios.h>
    #define MKDIR(dir) mkdir(dir, 0755)
#endif

using namespace std;

// Enhanced ItemRec structure with modern features
struct ItemRec { 
    string name; 
    string barcode;
    string category;
    string supplier;
    double rate = 0.0;
    int qty = 0; 
    int low_stock_threshold = 5;
    time_t last_updated = 0;
};

// Customer structure
struct Customer {
    string id;
    string name;
    string phone;
    string email;
    double loyalty_points = 0;
    double total_spent = 0;
    int visit_count = 0;
    time_t last_visit = 0;
};

// Payment method enumeration
enum PaymentMethod {
    CASH = 1,
    CARD = 2,
    MOBILE_BANKING = 3,
    DIGITAL_WALLET = 4
};

// Payment structure
struct Payment {
    PaymentMethod method;
    double amount;
    string reference_number;
    time_t timestamp;
};

// Configuration constants
const string INVENTORY_FILE = "Bill.txt";
const string SALES_FILE = "Sales.txt";
const string CUSTOMER_FILE = "customers.txt";
const string RECEIPT_FOLDER = "Receipts";
const string REPORT_FOLDER = "Reports";
const int LOW_STOCK_THRESHOLD = 5;
const double VAT_RATE = 0.05;
const double DISCOUNT_THRESHOLD = 500.0;
const double DISCOUNT_AMOUNT = 50.0;

// Function declarations
void display_inventory();
void search_inventory();
void delete_item_flow();
void generate_daily_report();
void view_sales_history();
void add_item_flow();
void print_bill_flow();
void improved_update_item_flow();
void customer_management_menu();
void scan_barcode_mode();
void show_dashboard();
void quick_sale_mode();
void generate_purchase_order();
void export_to_csv();

// Additional function declarations
vector<ItemRec> load_inventory();
bool save_inventory(const vector<ItemRec>& inv);
int find_item(const vector<ItemRec>& inv, const string &name);
void save_sale_record(const string& customer_name, double net_total, const vector<pair<ItemRec,int>>& items);
void print_receipt(const vector<pair<ItemRec,int>>& items, double total, double vat, 
                  double discount, double net_total, const vector<Payment>& payments,
                  const string& customer = "", Customer* customer_obj = nullptr);
void check_low_stock(const vector<ItemRec>& inv);

// Cross-platform console color support
void setColor(int color) {
    #ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    #else
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

// Progress bar for operations
void show_progress_bar(int percentage) {
    cout << "\r[";
    int pos = percentage / 2;
    for(int i = 0; i < 50; ++i) {
        if(i < pos) cout << "█";
        else cout << "░";
    }
    cout << "] " << percentage << "%";
    cout.flush();
}

// Get current date/time string
string get_current_datetime() {
    time_t t = time(0);
    tm* now = localtime(&t);
    char buf[30];
    strftime(buf, 30, "%Y-%m-%d %H:%M:%S", now);
    return string(buf);
}

// Generate unique ID
string generate_customer_id() {
    static int counter = 1000;
    return "CUST" + to_string(++counter);
}

// Simple directory creation function (more portable)
void ensure_directories() {
    MKDIR(RECEIPT_FOLDER.c_str());
    MKDIR(REPORT_FOLDER.c_str());
}

// Enhanced parsing with better error handling
bool parse_line(const string& line, ItemRec& out) {
    if(line.empty() || line[0] == '#') return false;
    
    vector<string> parts;
    stringstream ss(line);
    string item;
    
    while(getline(ss, item, '|')) {
        parts.push_back(item);
    }
    
    if(parts.size() < 3) return false;
    
    out.name = parts[0];
    out.name.erase(0, out.name.find_first_not_of(" \t"));
    out.name.erase(out.name.find_last_not_of(" \t") + 1);
    
    try { 
        out.rate = stod(parts[1]); 
        out.qty = stoi(parts[2]); 
        
        // Parse additional fields if available
        if(parts.size() > 3) out.barcode = parts[3];
        if(parts.size() > 4) out.category = parts[4];
        if(parts.size() > 5) out.supplier = parts[5];
        if(parts.size() > 6) out.low_stock_threshold = stoi(parts[6]);
        
    } catch (const exception& e) { 
        return false; 
    }
    
    return !out.name.empty() && out.rate >= 0 && out.qty >= 0;
}

string serialize_line(const ItemRec& r) { 
    return r.name + "|" + to_string(r.rate) + "|" + to_string(r.qty) + "|" +
           r.barcode + "|" + r.category + "|" + r.supplier + "|" + 
           to_string(r.low_stock_threshold);
}

// Customer management functions
// Around line 223, add error handling in load_customers()

vector<Customer> load_customers() {
    vector<Customer> customers;
    ifstream in(CUSTOMER_FILE);
    if(!in.is_open()) return customers;
    
    string line;
    while(getline(in, line)) {
        if(line.empty() || line[0] == '#') continue;
        
        stringstream ss(line);
        Customer customer;
        string temp;
        
        try {
            getline(ss, customer.id, '|');
            getline(ss, customer.name, '|');
            getline(ss, customer.phone, '|');
            getline(ss, customer.email, '|');
            getline(ss, temp, '|'); customer.loyalty_points = stod(temp);
            getline(ss, temp, '|'); customer.total_spent = stod(temp);
            getline(ss, temp, '|'); customer.visit_count = stoi(temp);
            getline(ss, temp); customer.last_visit = stoll(temp);
            
            customers.push_back(customer);
        } catch (const exception& e) {
            // Skip corrupted customer records
            continue;
        }
    }
    return customers;
}

bool save_customers(const vector<Customer>& customers) {
    ofstream out(CUSTOMER_FILE, ios::trunc);
    if(!out.is_open()) return false;
    
    out << "# Customer File - Format: ID|Name|Phone|Email|Points|TotalSpent|VisitCount|LastVisit\n";
    for(const auto& customer : customers) {
        out << customer.id << "|" << customer.name << "|" << customer.phone << "|"
            << customer.email << "|" << customer.loyalty_points << "|" << customer.total_spent
            << "|" << customer.visit_count << "|" << customer.last_visit << "\n";
    }
    return true;
}

Customer* find_customer_by_phone(vector<Customer>& customers, const string& phone) {
    for(auto& customer : customers) {
        if(customer.phone == phone) return &customer;
    }
    return nullptr;
}

// Barcode functions
bool validate_barcode(const string& barcode) {
    if(barcode.empty()) return true; // Allow empty barcodes
    regex barcode_pattern("^[0-9]{8,13}$");
    return regex_match(barcode, barcode_pattern);
}

ItemRec* find_item_by_barcode(vector<ItemRec>& inv, const string& barcode) {
    for(auto& item : inv) {
        if(item.barcode == barcode && !barcode.empty()) return &item;
    }
    return nullptr;
}

void scan_barcode_mode() {
    clearScreen();
    setColor(11);
    cout << "\n=== BARCODE SCANNER MODE ===\n";
    cout << "┌─────────────────────────────────────────────┐\n";
    cout << "│ Scan barcode or enter manually              │\n";
    cout << "│ (Press Enter without input to exit)        │\n";
    cout << "└─────────────────────────────────────────────┘\n\n";
    setColor(14);
    cout << "Barcode: ";
    setColor(7);
    
    string barcode;
    getline(cin, barcode);
    
    if(barcode.empty()) return;
    
    if(!validate_barcode(barcode)) {
        setColor(4);
        cout << "❌ Invalid barcode format! Must be 8-13 digits.\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    auto inv = load_inventory();
    ItemRec* item = find_item_by_barcode(inv, barcode);
    
    if(item) {
        setColor(10);
        cout << "✅ Found: " << item->name << "\n";
        cout << "   Rate: " << fixed << setprecision(2) << item->rate << " BDT\n";
        cout << "   Stock: " << item->qty << " units\n";
        cout << "   Category: " << (item->category.empty() ? "N/A" : item->category) << "\n";
        setColor(7);
    } else {
        setColor(4);
        cout << "❌ Item not found for barcode: " << barcode << "\n";
        setColor(7);
    }
    
    pauseSystem();
}

// Payment processing functions
string get_payment_method_name(PaymentMethod method) {
    switch(method) {
        case CASH: return "Cash";
        case CARD: return "Credit/Debit Card";
        case MOBILE_BANKING: return "Mobile Banking";
        case DIGITAL_WALLET: return "Digital Wallet";
        default: return "Unknown";
    }
}

vector<Payment> process_payment(double total_amount) {
    vector<Payment> payments;
    double remaining = total_amount;
    
    while(remaining > 0.01) {
        clearScreen();
        setColor(11);
        cout << "\n=== PAYMENT PROCESSING ===\n";
        cout << "┌─────────────────────────────────────────────┐\n";
        cout << "│ Remaining Amount: " << fixed << setprecision(2) << setw(22) << remaining << " BDT │\n";
        cout << "└─────────────────────────────────────────────┘\n\n";
        
        setColor(14);
        cout << "Payment Methods:\n";
        cout << "1. 💵 Cash\n";
        cout << "2. 💳 Credit/Debit Card\n";
        cout << "3. 📱 Mobile Banking (bKash/Nagad/Rocket)\n";
        cout << "4. 💰 Digital Wallet\n\n";
        cout << "Select payment method (1-4): ";
        setColor(7);
        
        int method;
        if(!(cin >> method) || method < 1 || method > 4) {
            wait_and_flush();
            setColor(4);
            cout << "Invalid payment method!\n";
            setColor(7);
            sleepMs(1500);
            continue;
        }
        
        setColor(14);
        cout << "Enter amount: ";
        setColor(7);
        double amount;
        if(!(cin >> amount) || amount <= 0) {
            wait_and_flush();
            setColor(4);
            cout << "Invalid amount!\n";
            setColor(7);
            sleepMs(1500);
            continue;
        }
        wait_and_flush();
        
        if(amount > remaining) amount = remaining;
        
        Payment payment;
        payment.method = static_cast<PaymentMethod>(method);
        payment.amount = amount;
        payment.timestamp = time(0);
        
        if(method != CASH) {
            setColor(14);
            cout << "Enter reference/transaction ID: ";
            setColor(7);
            getline(cin, payment.reference_number);
        }
        
        payments.push_back(payment);
        remaining -= amount;
        
        setColor(10);
        cout << "✅ Payment of " << amount << " BDT recorded via " 
             << get_payment_method_name(payment.method) << "\n";
        setColor(7);
        
        if(remaining > 0.01) {
            sleepMs(1500);
        }
    }
    
    return payments;
}

void print_payment_summary(const vector<Payment>& payments) {
    if(payments.empty()) return;
    
    setColor(11);
    cout << "\n=== PAYMENT SUMMARY ===\n";
    cout << "┌──────────────────────┬──────────┬─────────────────┐\n";
    cout << "│ Method               │ Amount   │ Reference       │\n";
    cout << "├──────────────────────┼──────────┼─────────────────┤\n";
    setColor(7);
    
    for(const auto& p : payments) {
        cout << "│ " << left << setw(20) << get_payment_method_name(p.method)
             << " │ " << right << setw(8) << fixed << setprecision(2) << p.amount 
             << " │ " << left << setw(15) << (p.reference_number.empty() ? "N/A" : p.reference_number.substr(0,15))
             << " │\n";
    }
    
    setColor(11);
    cout << "└──────────────────────┴──────────┴─────────────────┘\n";
    setColor(7);
}

vector<ItemRec> load_inventory() {
    vector<ItemRec> inv; 
    ifstream in(INVENTORY_FILE);
    if(!in.is_open()) {
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
    
    out << "# Enhanced Inventory File - Format: Name|Rate|Quantity|Barcode|Category|Supplier|LowStockThreshold\n";
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

// Enhanced receipt printing with payment details
void print_receipt(const vector<pair<ItemRec,int>>& items, double total, double vat, 
                  double discount, double net_total, const vector<Payment>& payments,
                  const string& customer, Customer* customer_obj) {
    setColor(11);
    cout << "\n" << string(70, '=') << "\n";
    cout << "                    SUPERMARKET RECEIPT\n";
    cout << string(70, '=') << "\n";
    setColor(7);
    
    if(!customer.empty()) {
        cout << "Customer: " << customer << "\n";
        if(customer_obj) {
            cout << "Phone: " << customer_obj->phone << "\n";
            cout << "Loyalty Points: " << fixed << setprecision(0) << customer_obj->loyalty_points << "\n";
        }
    }
    
    cout << "Date: " << get_current_datetime() << "\n\n";
    
    setColor(11);
    cout << "┌──────────────────┬───────┬─────┬──────────┐\n";
    cout << "│ Item             │ Rate  │ Qty │ Amount   │\n";
    cout << "├──────────────────┼───────┼─────┼──────────┤\n"; 
    setColor(7);
    
    for(const auto &p : items) {
        const auto &it = p.first; 
        int q = p.second;
        double amount = it.rate * q;
        
        cout << "│ " << left << setw(16) << it.name.substr(0, 16)
             << " │ " << right << setw(5) << fixed << setprecision(2) << it.rate
             << " │ " << setw(3) << q
             << " │ " << setw(8) << amount << " │\n";
    }
    
    setColor(11);
    cout << "└──────────────────┴───────┴─────┴──────────┘\n"; 
    setColor(14);
    cout << fixed << setprecision(2);
    cout << right << setw(50) << "Subtotal: " << setw(12) << total << " BDT\n";
    cout << right << setw(50) << "VAT(" << (VAT_RATE*100) << "%): " << setw(12) << vat << " BDT\n";
    if(discount > 0) {
        cout << right << setw(50) << "Discount: " << setw(12) << discount << " BDT\n";
    }
    setColor(11);
    cout << string(70, '-') << "\n";
    setColor(10);
    cout << right << setw(50) << "NET TOTAL: " << setw(12) << net_total << " BDT\n";
    setColor(11);
    cout << string(70, '=') << "\n";
    
    // Print payment summary
    print_payment_summary(payments);
    
    setColor(11);
    cout << string(70, '=') << "\n";
    setColor(7);
}

// Check for low stock items
void check_low_stock(const vector<ItemRec>& inv) {
    vector<ItemRec> low_stock_items;
    for(const auto& item : inv) {
        int threshold = item.low_stock_threshold > 0 ? item.low_stock_threshold : LOW_STOCK_THRESHOLD;
        if(item.qty <= threshold && item.qty > 0) {
            low_stock_items.push_back(item);
        }
    }
    
    if(!low_stock_items.empty()) {
        setColor(4);
        cout << "\n🚨 LOW STOCK ALERT! 🚨\n";
        setColor(14);
        cout << "The following items are running low:\n";
        setColor(7);
        for(const auto& item : low_stock_items) {
            cout << "📦 " << item.name << " (Only " << item.qty << " left)\n";
        }
        cout << "\n";
        sleepMs(3000);
    }
}

// Dashboard function
void show_dashboard() {
    clearScreen();
    auto inv = load_inventory();
    auto customers = load_customers();
    
    setColor(11);
    cout << "╔══════════════════ DASHBOARD ══════════════════╗\n";
    cout << "║              📊 System Overview                ║\n";
    cout << "╚════════════════════════════════════════════════╝\n";
    setColor(7);
    
    // Calculate statistics
    int total_items = inv.size();
    int out_of_stock = 0, low_stock = 0;
    double total_inventory_value = 0;
    
    for(const auto& item : inv) {
        total_inventory_value += item.rate * item.qty;
        if(item.qty == 0) out_of_stock++;
        else if(item.qty <= item.low_stock_threshold) low_stock++;
    }
    
    cout << "📦 Total Items in Inventory: " << total_items << "\n";
    cout << "💰 Total Inventory Value: " << fixed << setprecision(2) << total_inventory_value << " BDT\n";
    cout << "👥 Total Customers: " << customers.size() << "\n";
    cout << "⚠️  Low Stock Items: " << low_stock << "\n";
    cout << "❌ Out of Stock Items: " << out_of_stock << "\n";
    
    setColor(14);
    cout << "\n🚀 Quick Actions:\n";
    cout << "F1 - Quick Sale    F2 - Add Item    F3 - Reports\n";
    cout << "F4 - Barcode Scan  F5 - Customers   F6 - Dashboard\n";
    setColor(7);
    
    pauseSystem();
}

// Quick sale mode
// Replace the existing quick_sale_mode() function with this enhanced version

void quick_sale_mode() {
    clearScreen();
    setColor(11);
    cout << "\n=== ⚡ QUICK SALE MODE ===\n";
    setColor(14);
    cout << "Enter item name or barcode: ";
    setColor(7);
    
    string input;
    getline(cin, input);
    if(input.empty()) return;
    
    auto inv = load_inventory();
    ItemRec* item = nullptr;
    int idx = -1;
    
    // Check if it's a barcode first
    if(validate_barcode(input) && !input.empty()) {
        item = find_item_by_barcode(inv, input);
        if(item) {
            for(size_t i = 0; i < inv.size(); i++) {
                if(&inv[i] == item) {
                    idx = i;
                    break;
                }
            }
        }
    }
    
    // If not found by barcode, search by name
    if(!item) {
        idx = find_item(inv, input);
        if(idx >= 0) item = &inv[idx];
    }
    
    if(!item) {
        setColor(4);
        cout << "❌ Item not found!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    setColor(10);
    cout << "✅ Found: " << item->name << " (" << item->rate << " BDT)\n";
    setColor(14);
    cout << "Quantity (Available: " << item->qty << "): ";
    setColor(7);
    
    int qty;
    if(!(cin >> qty) || qty <= 0 || qty > item->qty) {
        wait_and_flush();
        setColor(4);
        cout << "❌ Invalid quantity!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    wait_and_flush();
    
    // Quick sale process
    double total = item->rate * qty;
    double vat = total * VAT_RATE;
    double discount = total > DISCOUNT_THRESHOLD ? DISCOUNT_AMOUNT : 0;
    double net_total = total + vat - discount;
    
    setColor(10);
    cout << "\n💰 Subtotal: " << fixed << setprecision(2) << total << " BDT\n";
    cout << "💰 VAT (5%): " << vat << " BDT\n";
    if(discount > 0) {
        cout << "💰 Discount: " << discount << " BDT\n";
    }
    cout << "💰 NET TOTAL: " << net_total << " BDT\n";
    setColor(14);
    cout << "\nProceed with quick sale? (y/n): ";
    setColor(7);
    
    char confirm;
    cin >> confirm;
    wait_and_flush();
    
    if(confirm == 'y' || confirm == 'Y') {
        // Process payment
        vector<Payment> payments = process_payment(net_total);
        
        // Update inventory
        inv[idx].qty -= qty;
        save_inventory(inv);
        
        // **NEW: Save sales history**
        vector<pair<ItemRec,int>> sold_items;
        sold_items.push_back({*item, qty});
        save_sale_record("Quick Sale Customer", net_total, sold_items);
        
        // **NEW: Generate quick receipt**
        clearScreen();
        setColor(11);
        cout << "\n=== ⚡ QUICK SALE RECEIPT ===\n";
        setColor(7);
        print_receipt(sold_items, total, vat, discount, net_total, payments, "Quick Sale Customer", nullptr);
        
        setColor(10);
        cout << "\n✅ Quick sale completed and recorded!\n";
        setColor(14);
        cout << "📄 Sales history updated\n";
        cout << "🧾 Receipt generated\n";
        setColor(7);
        sleepMs(3000);
    } else {
        setColor(14);
        cout << "Quick sale cancelled.\n";
        setColor(7);
        sleepMs(1500);
    }
}

// Customer management
void customer_management_menu() {
    bool exit = false;
    while(!exit) {
        clearScreen();
        setColor(11);
        cout << "\n=== 👥 CUSTOMER MANAGEMENT ===\n";
        cout << "┌─────────────────────────────────────┐\n";
        cout << "│ 1. Add New Customer                 │\n";
        cout << "│ 2. Search Customer                  │\n";
        cout << "│ 3. View All Customers               │\n";
        cout << "│ 4. Customer Purchase History        │\n";
        cout << "│ 5. Back to Main Menu                │\n";
        cout << "└─────────────────────────────────────┘\n";
        cout << "Enter choice: ";
        setColor(7);
        
        int choice;
        if(!(cin >> choice)) {
            wait_and_flush();
            continue;
        }
        wait_and_flush();
        
        switch(choice) {
            case 1: {
                Customer customer;
                setColor(14);
                cout << "Enter customer phone: ";
                setColor(7);
                getline(cin, customer.phone);
                
                auto customers = load_customers();
                if(find_customer_by_phone(customers, customer.phone)) {
                    setColor(4);
                    cout << "❌ Customer already exists!\n";
                    setColor(7);
                    sleepMs(1500);
                    break;
                }
                
                setColor(14);
                cout << "Enter customer name: ";
                setColor(7);
                getline(cin, customer.name);
                cout << "Enter email (optional): ";
                getline(cin, customer.email);
                
                customer.id = generate_customer_id();
                customer.last_visit = time(0);
                
                customers.push_back(customer);
                save_customers(customers);
                
                setColor(10);
                cout << "✅ Customer added! ID: " << customer.id << "\n";
                setColor(7);
                sleepMs(2000);
                break;
            }
            case 2: {
                setColor(14);
                cout << "Enter phone number to search: ";
                setColor(7);
                string phone;
                getline(cin, phone);
                
                auto customers = load_customers();
                Customer* customer = find_customer_by_phone(customers, phone);
                
                if(customer) {
                    setColor(10);
                    cout << "\n✅ Customer Found:\n";
                    setColor(7);
                    cout << "ID: " << customer->id << "\n";
                    cout << "Name: " << customer->name << "\n";
                    cout << "Phone: " << customer->phone << "\n";
                    cout << "Email: " << customer->email << "\n";
                    cout << "Loyalty Points: " << customer->loyalty_points << "\n";
                    cout << "Total Spent: " << customer->total_spent << " BDT\n";
                    cout << "Visit Count: " << customer->visit_count << "\n";
                } else {
                    setColor(4);
                    cout << "❌ Customer not found!\n";
                    setColor(7);
                }
                pauseSystem();
                break;
            }
            case 3: {
                auto customers = load_customers();
                if(customers.empty()) {
                    setColor(4);
                    cout << "No customers found!\n";
                    setColor(7);
                    sleepMs(1500);
                    break;
                }
                
                clearScreen();
                setColor(11);
                cout << "\n=== ALL CUSTOMERS ===\n";
                cout << "┌─────────┬────────────────┬─────────────┬───────┬────────────┐\n";
                cout << "│ ID      │ Name           │ Phone       │ Points│ Total Spent│\n";
                cout << "├─────────┼────────────────┼─────────────┼───────┼────────────┤\n";
                setColor(7);
                
                for(const auto& customer : customers) {
                    cout << "│ " << left << setw(7) << customer.id
                         << " │ " << setw(14) << customer.name.substr(0,14)
                         << " │ " << setw(11) << customer.phone
                         << " │ " << right << setw(5) << (int)customer.loyalty_points
                         << " │ " << setw(10) << fixed << setprecision(2) << customer.total_spent << " │\n";
                }
                
                setColor(11);
                cout << "└─────────┴────────────────┴─────────────┴───────┴────────────┘\n";
                setColor(7);
                pauseSystem();
                break;
            }
            case 5:
                exit = true;
                break;
            default:
                setColor(4);
                cout << "Invalid choice!\n";
                setColor(7);
                sleepMs(1000);
        }
    }
}

// Generate purchase order
void generate_purchase_order() {
    auto inv = load_inventory();
    if(inv.empty()) {
        setColor(4);
        cout << "No inventory found!\n";
        setColor(7);
        sleepMs(1500);
        return;
    }
    
    string filename = REPORT_FOLDER + "/PurchaseOrder_" + get_current_datetime().substr(0,10) + ".txt";
    ofstream po(filename);
    
    po << "=== PURCHASE ORDER ===\n";
    po << "Date: " << get_current_datetime() << "\n\n";
    po << "Items requiring restock:\n";
    po << string(60, '-') << "\n";
    
    bool has_items = false;
    for(const auto& item : inv) {
        int threshold = item.low_stock_threshold > 0 ? item.low_stock_threshold : LOW_STOCK_THRESHOLD;
        if(item.qty <= threshold) {
            int suggested_qty = threshold * 3; // Suggest 3x threshold
            po << "Item: " << item.name << "\n";
            po << "  Current Stock: " << item.qty << "\n";
            po << "  Suggested Order: " << suggested_qty << "\n";
            po << "  Supplier: " << (item.supplier.empty() ? "TBD" : item.supplier) << "\n";
            po << "  Category: " << (item.category.empty() ? "General" : item.category) << "\n\n";
            has_items = true;
        }
    }
    
    if(!has_items) {
        po << "No items require restocking at this time.\n";
    }
    
    po.close();
    
    setColor(10);
    cout << "✅ Purchase order generated: " << filename << "\n";
    setColor(7);
    sleepMs(2000);
}

// Export to CSV
void export_to_csv() {
    auto inv = load_inventory();
    string filename = REPORT_FOLDER + "/InventoryExport_" + get_current_datetime().substr(0,10) + ".csv";
    ofstream csv(filename);
    
    csv << "Name,Barcode,Rate,Quantity,Category,Supplier,LowStockThreshold\n";
    for(const auto& item : inv) {
        csv << "\"" << item.name << "\",\"" << item.barcode << "\"," 
            << item.rate << "," << item.qty << ",\"" << item.category 
            << "\",\"" << item.supplier << "\"," << item.low_stock_threshold << "\n";
    }
    csv.close();
    
    setColor(10);
    cout << "✅ Data exported to: " << filename << "\n";
    setColor(7);
    sleepMs(2000);
}

// Enhanced display inventory with new fields
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
    cout << "┌────┬──────────────────┬──────────┬───────┬─────────────┬────────────┬────────┐\n";
    cout << "│ No │ Item             │ Rate     │ Qty   │ Barcode     │ Category   │ Status │\n";
    cout << "├────┼──────────────────┼──────────┼───────┼─────────────┼────────────┼────────┤\n"; 
    setColor(7);
    
    for(size_t i = 0; i < inv.size(); i++) {
        const auto &it = inv[i];
        cout << "│ " << right << setw(2) << (i+1)
             << " │ " << left << setw(16) << it.name.substr(0, 16)
             << " │ " << right << setw(8) << fixed << setprecision(2) << it.rate
             << " │ " << setw(5) << it.qty 
             << " │ " << left << setw(11) << it.barcode.substr(0,11)
             << " │ " << setw(10) << it.category.substr(0,10) << " │ ";
        
        int threshold = it.low_stock_threshold > 0 ? it.low_stock_threshold : LOW_STOCK_THRESHOLD;
        if(it.qty == 0) {
            setColor(4); cout << " OUT   ";
        } else if(it.qty <= threshold) {
            setColor(14); cout << " LOW   ";
        } else {
            setColor(10); cout << " OK    ";
        }
        setColor(7);
        cout << " │\n";
    }
    
    setColor(11); 
    cout << "└────┴──────────────────┴──────────┴───────┴─────────────┴────────────┴────────┘\n"; 
    cout << "\nTotal Items: " << inv.size() << "\n";
    setColor(7); 
    pauseSystem();
}

// Enhanced search with category and barcode support
// Replace the existing search_inventory() function

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
    cout << "Search by:\n1. Name\n2. Category\n3. Barcode\nChoose option: "; 
    setColor(7); 
    
    int option;
    if(!(cin >> option)) {
        wait_and_flush();
        return;
    }
    wait_and_flush();
    
    string term;
    setColor(14);
    switch(option) {
        case 1: cout << "Enter item name: "; break;
        case 2: cout << "Enter category: "; break;
        case 3: cout << "Enter barcode: "; break;
        default: 
            setColor(4);
            cout << "Invalid option!\n";
            setColor(7);
            sleepMs(1500);
            return;
    }
    setColor(7);
    getline(cin, term);
    
    if(term.empty()) {
        setColor(4);
        cout << "Search term cannot be empty!\n";
        setColor(7);
        sleepMs(1500);
        return;
    }
    
    transform(term.begin(), term.end(), term.begin(), ::tolower);
    
    vector<ItemRec> results; 
    for(const auto &it : inv) {
        bool match = false;
        switch(option) {
            case 1: {
                string item_name = it.name;
                transform(item_name.begin(), item_name.end(), item_name.begin(), ::tolower);
                match = item_name.find(term) != string::npos;
                break;
            }
            case 2: {
                string category = it.category;
                transform(category.begin(), category.end(), category.begin(), ::tolower);
                match = category.find(term) != string::npos;
                break;
            }
            case 3: {
                match = it.barcode == term;
                break;
            }
        }
        if(match) results.push_back(it);
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
    cout << "┌────┬──────────────────┬──────────┬───────┬─────────────┬────────────┬────────┐\n";
    cout << "│ No │ Item             │ Rate     │ Qty   │ Barcode     │ Category   │ Status │\n";
    cout << "├────┼──────────────────┼──────────┼───────┼─────────────┼────────────┼────────┤\n"; 
    setColor(7);
    
    for(size_t i = 0; i < results.size(); i++) {
        const auto &it = results[i];
        cout << "│ " << right << setw(2) << (i+1)
             << " │ " << left << setw(16) << it.name.substr(0, 16)
             << " │ " << right << setw(8) << fixed << setprecision(2) << it.rate
             << " │ " << setw(5) << it.qty 
             << " │ " << left << setw(11) << it.barcode.substr(0,11)
             << " │ " << setw(10) << it.category.substr(0,10) << " │ ";
        
        int threshold = it.low_stock_threshold > 0 ? it.low_stock_threshold : LOW_STOCK_THRESHOLD;
        if(it.qty == 0) {
            setColor(4); cout << " OUT   ";
        } else if(it.qty <= threshold) {
            setColor(14); cout << " LOW   ";
        } else {
            setColor(10); cout << " OK    ";
        }
        setColor(7);
        cout << " │\n";
    }
    
    setColor(11); 
    cout << "└────┴──────────────────┴──────────┴───────┴─────────────┴────────────┴────────┘\n"; 
    cout << "\nFound " << results.size() << " item(s) matching '" << term << "'\n";
    setColor(7); 
    pauseSystem();
}

// Enhanced add item with new fields
void add_item_flow() {
    bool close = false;
    while(!close) {
        clearScreen(); 
        setColor(11); 
        cout << "\n\t=== INVENTORY MANAGEMENT ===\n";
        cout << "\t1. Add New Item\n\t2. Update Existing Item\n\t3. View Current Stock\n\t4. Barcode Scanner\n\t5. Back to Main Menu\n";
        cout << "\tEnter Choice: "; 
        setColor(7);
        
        int choice; 
        if(!(cin >> choice)) { 
            wait_and_flush(); 
            continue; 
        } 
        wait_and_flush();
        
        if(choice == 1) {
            ItemRec new_item;
            
            setColor(14); 
            cout << "\tEnter Item Name: "; 
            setColor(7); 
            getline(cin, new_item.name);
            
            if(new_item.name.empty()) {
                setColor(4); 
                cout << "Item name cannot be empty!\n"; 
                setColor(7); 
                sleepMs(1500); 
                continue;
            }
            
            setColor(14); 
            cout << "\tEnter Barcode (optional): "; 
            setColor(7); 
            getline(cin, new_item.barcode);
            
            if(!new_item.barcode.empty() && !validate_barcode(new_item.barcode)) {
                setColor(4); 
                cout << "Invalid barcode format!\n"; 
                setColor(7); 
                sleepMs(1500); 
                continue;
            }
            
            setColor(14); 
            cout << "\tEnter Category (optional): "; 
            setColor(7); 
            getline(cin, new_item.category);
            
            setColor(14); 
            cout << "\tEnter Supplier (optional): "; 
            setColor(7); 
            getline(cin, new_item.supplier);
            
            setColor(14); 
            cout << "\tEnter Rate (BDT): "; 
            setColor(7); 
            if(!(cin >> new_item.rate) || new_item.rate < 0) {
                setColor(4); 
                cout << "Invalid rate!\n"; 
                setColor(7); 
                wait_and_flush();
                sleepMs(1500); 
                continue;
            }
            
            setColor(14); 
            cout << "\tEnter Quantity: "; 
            setColor(7); 
            if(!(cin >> new_item.qty) || new_item.qty < 0) {
                setColor(4); 
                cout << "Invalid quantity!\n"; 
                setColor(7); 
                wait_and_flush();
                sleepMs(1500); 
                continue;
            }
            
            setColor(14); 
            cout << "\tEnter Low Stock Threshold (default 5): "; 
            setColor(7); 
            cin >> new_item.low_stock_threshold;
            if(new_item.low_stock_threshold <= 0) {
                new_item.low_stock_threshold = LOW_STOCK_THRESHOLD;
            }
            wait_and_flush();
            
            new_item.last_updated = time(0);
            
            auto inv = load_inventory(); 
            int idx = find_item(inv, new_item.name);
            
            if(idx >= 0) {
                setColor(14);
                cout << "Item exists! Updating existing item...\n";
                inv[idx] = new_item;
                setColor(7);
            } else {
                inv.push_back(new_item);
            }
            
            if(!save_inventory(inv)) {
                setColor(4);
                cout << "Error: Cannot save inventory!\n";
                setColor(7);
            } else {
                setColor(10);
                cout << "✅ Item successfully " << (idx >= 0 ? "updated" : "added") << "!\n";
                setColor(7);
            }
            sleepMs(2000);
            
        } else if(choice == 2) {
            improved_update_item_flow();
        } else if(choice == 3) {
            display_inventory();
        } else if(choice == 4) {
            scan_barcode_mode();
        } else if(choice == 5) {
            close = true;
        } else {
            setColor(4);
            cout << "Invalid choice!\n";
            setColor(7);
            sleepMs(1000);
        }
    }
}

// Enhanced billing with customer and payment integration
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
    
    // Customer selection
    string customer_name; 
    Customer* customer_obj = nullptr;
    auto customers = load_customers();
    
    setColor(14); 
    cout << "Enter customer phone (or press Enter for walk-in): "; 
    setColor(7); 
    string phone;
    getline(cin, phone);
    
    if(!phone.empty()) {
        customer_obj = find_customer_by_phone(customers, phone);
        if(customer_obj) {
            customer_name = customer_obj->name;
            customer_obj->visit_count++;
            customer_obj->last_visit = time(0);
            setColor(10);
            cout << "✅ Customer found: " << customer_name << " (Points: " << customer_obj->loyalty_points << ")\n";
            setColor(7);
        } else {
            setColor(14);
            cout << "Customer not found. Enter name for new customer: ";
            setColor(7);
            getline(cin, customer_name);
            if(!customer_name.empty()) {
                Customer new_customer;
                new_customer.id = generate_customer_id();
                new_customer.name = customer_name;
                new_customer.phone = phone;
                new_customer.visit_count = 1;
                new_customer.last_visit = time(0);
                customers.push_back(new_customer);
                customer_obj = &customers.back();
                customer_name = new_customer.name;
            }
        }
    }
    
    if(customer_name.empty()) customer_name = "Walk-in Customer";

    vector<pair<ItemRec,int>> sold_items; 
    double total = 0; 
    bool close = false;
    
    while(!close) {
        clearScreen(); 
        setColor(11);
        cout << "\n\t=== 🛒 BILLING SYSTEM ===\n";
        cout << "\tCustomer: " << customer_name << "\n";
        if(customer_obj) {
            cout << "\tLoyalty Points: " << customer_obj->loyalty_points << "\n";
        }
        cout << "\tCurrent Bill Total: " << fixed << setprecision(2) << total << " BDT\n\n";
        cout << "\t1. Add Item to Bill\n\t2. Add by Barcode\n\t3. View Current Bill\n\t4. Apply Loyalty Discount\n\t5. Complete Sale\n\t6. Cancel Sale\n";
        cout << "\tEnter Choice: "; 
        setColor(7);
        
        int choice; 
        if(!(cin >> choice)) { 
            wait_and_flush(); 
            continue; 
        } 
        wait_and_flush();
        
        if(choice == 1 || choice == 2) {
            string input; 
            int q; 
            
            setColor(14); 
            if(choice == 1) {
                cout << "Item name: "; 
            } else {
                cout << "Barcode: ";
            }
            setColor(7); 
            getline(cin, input);
            
            if(input.empty()) {
                setColor(4); 
                cout << "Input cannot be empty!\n"; 
                setColor(7); 
                sleepMs(1500); 
                continue;
            }
            
            setColor(14); 
            cout << "Quantity: "; 
            setColor(7); 
            if(!(cin >> q) || q <= 0) { 
                setColor(4); 
                cout << "Invalid quantity!\n"; 
                setColor(7); 
                wait_and_flush(); 
                sleepMs(1500); 
                continue; 
            }
            wait_and_flush();
            
            int idx = -1;
            if(choice == 2) {
                // Search by barcode
                for(size_t i = 0; i < inv.size(); i++) {
                    if(inv[i].barcode == input) {
                        idx = i;
                        break;
                    }
                }
            } else {
                // Search by name
                idx = find_item(inv, input);
            }
            
            if(idx < 0) { 
                setColor(4); 
                cout << "Item not found!\n"; 
                setColor(7); 
                sleepMs(2000); 
                continue; 
            }
            
            if(inv[idx].qty < q) { 
                setColor(4); 
                cout << "Insufficient stock! Only " << inv[idx].qty << " available.\n"; 
                setColor(7); 
                sleepMs(2000); 
                continue; 
            }
            
            inv[idx].qty -= q; 
            sold_items.push_back({inv[idx], q}); 
            total += inv[idx].rate * q;
            save_inventory(inv);
            
            setColor(10);
            cout << "✅ Added " << q << " x " << inv[idx].name << " (BDT " << (inv[idx].rate * q) << ")\n";
            setColor(7);
            sleepMs(1500);
            
        } else if(choice == 3) {
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
            vector<Payment> empty_payments; // Temporary for preview
            print_receipt(sold_items, total, vat, discount, net_total, empty_payments, customer_name, customer_obj);
            pauseSystem();
            
        } else if(choice == 4) {
            if(customer_obj && customer_obj->loyalty_points > 0) {
                double max_discount = total * 0.1; // Max 10% discount
                double points_discount = min(customer_obj->loyalty_points * 0.01, max_discount);
                
                setColor(14);
                cout << "Available loyalty discount: " << points_discount << " BDT\n";
                cout << "Apply discount? (y/n): ";
                setColor(7);
                
                char apply;
                cin >> apply;
                wait_and_flush();
                
                if(apply == 'y' || apply == 'Y') {
                    total -= points_discount;
                    customer_obj->loyalty_points -= points_discount * 100;
                    setColor(10);
                    cout << "✅ Loyalty discount of " << points_discount << " BDT applied!\n";
                    setColor(7);
                    sleepMs(2000);
                }
            } else {
                setColor(4);
                cout << "No loyalty points available!\n";
                setColor(7);
                sleepMs(1500);
            }
            
        } else if(choice == 5) {
            if(sold_items.empty()) {
                setColor(4);
                cout << "Cannot complete sale - no items in bill!\n";
                setColor(7);
                sleepMs(1500);
                continue;
            }
            close = true;
        } else if(choice == 6) {
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
    
    // Process payment
    vector<Payment> payments = process_payment(net_total);
    
    // Update customer data
    if(customer_obj) {
        customer_obj->total_spent += net_total;
        customer_obj->loyalty_points += net_total * 0.01; // 1% as loyalty points
        save_customers(customers);
    }
    
    clearScreen(); 
    setColor(13); 
    cout << "\n\t==== FINAL RECEIPT ====\n"; 
    setColor(7);
    print_receipt(sold_items, total, vat, discount, net_total, payments, customer_name, customer_obj);

    // Save sale record
    save_sale_record(customer_name, net_total, sold_items);
    
    setColor(10); 
    cout << "\n✅ Sale completed successfully!\n"; 
    setColor(7); 
    
    check_low_stock(inv);
    sleepMs(3000);
}

// Add this function before main()

void save_sale_record(const string& customer_name, double net_total, const vector<pair<ItemRec,int>>& items) {
    ofstream sales_file(SALES_FILE, ios::app);
    if(!sales_file.is_open()) return;
    
    // Create header if file is new
    ifstream check_file(SALES_FILE);
    check_file.seekg(0, ios::end);
    if(check_file.tellg() == 0) {
        sales_file << "# Sales History - Format: DateTime|Customer|Amount|Items\n";
    }
    check_file.close();
    
    string items_list = "";
    for(size_t i = 0; i < items.size(); i++) {
        items_list += items[i].first.name + "(" + to_string(items[i].second) + ")";
        if(i < items.size() - 1) items_list += ",";
    }
    
    sales_file << get_current_datetime() << "|" << customer_name << "|" 
               << fixed << setprecision(2) << net_total << "|" << items_list << "\n";
    sales_file.close();
}

// Add this function before main()

void improved_update_item_flow() {
    clearScreen();
    auto inv = load_inventory();
    if(inv.empty()) {
        setColor(4);
        cout << "Inventory is empty!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    setColor(11);
    cout << "\n=== UPDATE ITEM ===\n";
    setColor(14);
    cout << "Search by:\n1. Item Name\n2. Barcode\nChoose option: ";
    setColor(7);
    
    int option;
    if(!(cin >> option) || (option != 1 && option != 2)) {
        wait_and_flush();
        setColor(4);
        cout << "Invalid option!\n";
        setColor(7);
        sleepMs(1500);
        return;
    }
    wait_and_flush();
    
    string search_term;
    setColor(14);
    if(option == 1) {
        cout << "Enter item name: ";
    } else {
        cout << "Enter barcode: ";
    }
    setColor(7);
    getline(cin, search_term);
    
    if(search_term.empty()) {
        setColor(4);
        cout << "Search term cannot be empty!\n";
        setColor(7);
        sleepMs(1500);
        return;
    }
    
    int idx = -1;
    if(option == 1) {
        idx = find_item(inv, search_term);
    } else {
        for(size_t i = 0; i < inv.size(); i++) {
            if(inv[i].barcode == search_term) {
                idx = i;
                break;
            }
        }
    }
    
    if(idx < 0) {
        setColor(4);
        cout << "Item not found!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    setColor(10);
    cout << "\n✅ Found Item: " << inv[idx].name << "\n";
    cout << "Current Details:\n";
    cout << "Rate: " << inv[idx].rate << " BDT\n";
    cout << "Quantity: " << inv[idx].qty << "\n";
    cout << "Barcode: " << (inv[idx].barcode.empty() ? "None" : inv[idx].barcode) << "\n";
    cout << "Category: " << (inv[idx].category.empty() ? "None" : inv[idx].category) << "\n";
    cout << "Supplier: " << (inv[idx].supplier.empty() ? "None" : inv[idx].supplier) << "\n";
    setColor(7);
    
    cout << "\nWhat would you like to update?\n";
    cout << "1. Rate\n2. Quantity\n3. Barcode\n4. Category\n5. Supplier\n6. Low Stock Threshold\n7. All Details\n";
    cout << "Enter choice: ";
    
    int update_choice;
    if(!(cin >> update_choice)) {
        wait_and_flush();
        return;
    }
    wait_and_flush();
    
    switch(update_choice) {
        case 1: {
            setColor(14);
            cout << "Enter new rate: ";
            setColor(7);
            double new_rate;
            if(cin >> new_rate && new_rate >= 0) {
                inv[idx].rate = new_rate;
                setColor(10);
                cout << "✅ Rate updated!\n";
                setColor(7);
            } else {
                setColor(4);
                cout << "Invalid rate!\n";
                setColor(7);
            }
            wait_and_flush();
            break;
        }
        case 2: {
            setColor(14);
            cout << "Enter new quantity: ";
            setColor(7);
            int new_qty;
            if(cin >> new_qty && new_qty >= 0) {
                inv[idx].qty = new_qty;
                setColor(10);
                cout << "✅ Quantity updated!\n";
                setColor(7);
            } else {
                setColor(4);
                cout << "Invalid quantity!\n";
                setColor(7);
            }
            wait_and_flush();
            break;
        }
        case 3: {
            setColor(14);
            cout << "Enter new barcode: ";
            setColor(7);
            string new_barcode;
            getline(cin, new_barcode);
            if(validate_barcode(new_barcode)) {
                inv[idx].barcode = new_barcode;
                setColor(10);
                cout << "✅ Barcode updated!\n";
                setColor(7);
            } else {
                setColor(4);
                cout << "Invalid barcode format!\n";
                setColor(7);
            }
            break;
        }
        case 4: {
            setColor(14);
            cout << "Enter new category: ";
            setColor(7);
            getline(cin, inv[idx].category);
            setColor(10);
            cout << "✅ Category updated!\n";
            setColor(7);
            break;
        }
        case 5: {
            setColor(14);
            cout << "Enter new supplier: ";
            setColor(7);
            getline(cin, inv[idx].supplier);
            setColor(10);
            cout << "✅ Supplier updated!\n";
            setColor(7);
            break;
        }
        case 6: {
            setColor(14);
            cout << "Enter new low stock threshold: ";
            setColor(7);
            int threshold;
            if(cin >> threshold && threshold > 0) {
                inv[idx].low_stock_threshold = threshold;
                setColor(10);
                cout << "✅ Low stock threshold updated!\n";
                setColor(7);
            } else {
                setColor(4);
                cout << "Invalid threshold!\n";
                setColor(7);
            }
            wait_and_flush();
            break;
        }
        case 7: {
            // Update all details
            setColor(14);
            cout << "Enter new rate: ";
            setColor(7);
            cin >> inv[idx].rate;
            
            setColor(14);
            cout << "Enter new quantity: ";
            setColor(7);
            cin >> inv[idx].qty;
            wait_and_flush();
            
            setColor(14);
            cout << "Enter new barcode: ";
            setColor(7);
            getline(cin, inv[idx].barcode);
            
            setColor(14);
            cout << "Enter new category: ";
            setColor(7);
            getline(cin, inv[idx].category);
            
            setColor(14);
            cout << "Enter new supplier: ";
            setColor(7);
            getline(cin, inv[idx].supplier);
            
            setColor(14);
            cout << "Enter new low stock threshold: ";
            setColor(7);
            cin >> inv[idx].low_stock_threshold;
            wait_and_flush();
            
            setColor(10);
            cout << "✅ All details updated!\n";
            setColor(7);
            break;
        }
        default:
            setColor(4);
            cout << "Invalid choice!\n";
            setColor(7);
            sleepMs(1500);
            return;
    }
    
    inv[idx].last_updated = time(0);
    
    if(save_inventory(inv)) {
        setColor(10);
        cout << "✅ Changes saved successfully!\n";
        setColor(7);
    } else {
        setColor(4);
        cout << "❌ Error saving changes!\n";
        setColor(7);
    }
    
    sleepMs(2000);
}

// Add this function before main()

void delete_item_flow() {
    clearScreen();
    auto inv = load_inventory();
    if(inv.empty()) {
        setColor(4);
        cout << "Inventory is empty!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    setColor(11);
    cout << "\n=== DELETE ITEM ===\n";
    setColor(14);
    cout << "Search item to delete:\n1. By Name\n2. By Barcode\nChoose option: ";
    setColor(7);
    
    int option;
    if(!(cin >> option) || (option != 1 && option != 2)) {
        wait_and_flush();
        setColor(4);
        cout << "Invalid option!\n";
        setColor(7);
        sleepMs(1500);
        return;
    }
    wait_and_flush();
    
    string search_term;
    setColor(14);
    if(option == 1) {
        cout << "Enter item name to delete: ";
    } else {
        cout << "Enter barcode: ";
    }
    setColor(7);
    getline(cin, search_term);
    
    if(search_term.empty()) {
        setColor(4);
        cout << "Search term cannot be empty!\n";
        setColor(7);
        sleepMs(1500);
        return;
    }
    
    int idx = -1;
    if(option == 1) {
        idx = find_item(inv, search_term);
    } else {
        for(size_t i = 0; i < inv.size(); i++) {
            if(inv[i].barcode == search_term) {
                idx = i;
                break;
            }
        }
    }
    
    if(idx < 0) {
        setColor(4);
        cout << "❌ Item not found!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    setColor(14);
    cout << "\n⚠️  Item to delete:\n";
    cout << "Name: " << inv[idx].name << "\n";
    cout << "Rate: " << inv[idx].rate << " BDT\n";
    cout << "Quantity: " << inv[idx].qty << "\n";
    cout << "Category: " << (inv[idx].category.empty() ? "None" : inv[idx].category) << "\n";
    
    setColor(4);
    cout << "\nAre you sure you want to delete this item? (y/n): ";
    setColor(7);
    
    char confirm;
    cin >> confirm;
    wait_and_flush();
    
    if(confirm == 'y' || confirm == 'Y') {
        string deleted_name = inv[idx].name;
        inv.erase(inv.begin() + idx);
        
        if(save_inventory(inv)) {
            setColor(10);
            cout << "✅ Item '" << deleted_name << "' deleted successfully!\n";
            setColor(7);
        } else {
            setColor(4);
            cout << "❌ Error deleting item!\n";
            setColor(7);
        }
    } else {
        setColor(14);
        cout << "Delete operation cancelled.\n";
        setColor(7);
    }
    
    sleepMs(2000);
}

// Add this function before main()

void view_sales_history() {
    clearScreen();
    setColor(11);
    cout << "\n=== SALES HISTORY ===\n";
    setColor(7);
    
    ifstream sales_file(SALES_FILE);
    if(!sales_file.is_open()) {
        setColor(4);
        cout << "❌ No sales history found!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    vector<string> sales_records;
    string line;
    
    while(getline(sales_file, line)) {
        if(!line.empty() && line[0] != '#') {
            sales_records.push_back(line);
        }
    }
    sales_file.close();
    
    if(sales_records.empty()) {
        setColor(4);
        cout << "❌ No sales records found!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    setColor(14);
    cout << "Sales History Options:\n";
    cout << "1. View All Sales\n";
    cout << "2. View Today's Sales\n";
    cout << "3. Search by Date\n";
    cout << "4. Sales Summary\n";
    cout << "Enter choice: ";
    setColor(7);
    
    int choice;
    if(!(cin >> choice)) {
        wait_and_flush();
        return;
    }
    wait_and_flush();
    
    switch(choice) {
        case 1: {
            clearScreen();
            setColor(11);
            cout << "\n=== ALL SALES RECORDS ===\n";
            cout << "┌──────────────────────┬────────────────┬─────────────┐\n";
            cout << "│ Date & Time          │ Customer       │ Amount (BDT)│\n";
            cout << "├──────────────────────┼────────────────┼─────────────┤\n";
            setColor(7);
            
            for(const auto& record : sales_records) {
                // Parse and display sales record
                // Format: Date|Customer|Amount|Items
                stringstream ss(record);
                string date, customer, amount;
                getline(ss, date, '|');
                getline(ss, customer, '|');
                getline(ss, amount, '|');
                
                cout << "│ " << left << setw(20) << date.substr(0,20)
                     << " │ " << setw(14) << customer.substr(0,14)
                     << " │ " << right << setw(11) << amount << " │\n";
            }
            
            setColor(11);
            cout << "└──────────────────────┴────────────────┴─────────────┘\n";
            setColor(7);
            cout << "\nTotal Records: " << sales_records.size() << "\n";
            break;
        }
        case 2: {
            string today = get_current_datetime().substr(0, 10);
            int today_count = 0;
            double today_total = 0;
            
            clearScreen();
            setColor(11);
            cout << "\n=== TODAY'S SALES (" << today << ") ===\n";
            setColor(7);
            
            for(const auto& record : sales_records) {
                if(record.find(today) == 0) {
                    stringstream ss(record);
                    string date, customer, amount;
                    getline(ss, date, '|');
                    getline(ss, customer, '|');
                    getline(ss, amount, '|');
                    
                    cout << "🛒 " << customer << " - " << amount << " BDT at " 
                         << date.substr(11) << "\n";
                    
                    today_count++;
                    try {
                        today_total += stod(amount);
                    } catch (const exception& e) {
                        // Skip invalid amount records
                    }
                }
            }
            
            if(today_count == 0) {
                setColor(4);
                cout << "No sales recorded for today.\n";
                setColor(7);
            } else {
                setColor(10);
                cout << "\n📊 Today's Summary:\n";
                cout << "Total Sales: " << today_count << "\n";
                cout << "Total Revenue: " << fixed << setprecision(2) << today_total << " BDT\n";
                cout << "Average Sale: " << (today_total / today_count) << " BDT\n";
                setColor(7);
            }
            break;
        }
        case 3: {
            setColor(14);
            cout << "Enter date (YYYY-MM-DD): ";
            setColor(7);
            string search_date;
            getline(cin, search_date);
            
            int found_count = 0;
            double date_total = 0;
            
            clearScreen();
            setColor(11);
            cout << "\n=== SALES FOR " << search_date << " ===\n";
            setColor(7);
            
            for(const auto& record : sales_records) {
                if(record.find(search_date) == 0) {
                    stringstream ss(record);
                    string date, customer, amount;
                    getline(ss, date, '|');
                    getline(ss, customer, '|');
                    getline(ss, amount, '|');
                    
                    cout << "🛒 " << customer << " - " << amount << " BDT at " 
                         << date.substr(11) << "\n";
                    
                    found_count++;
                    try {
                        date_total += stod(amount);
                    } catch (const exception& e) {
                        // Skip invalid amount records
                    }
                }
            }
            
            if(found_count == 0) {
                setColor(4);
                cout << "No sales found for " << search_date << "\n";
                setColor(7);
            } else {
                setColor(10);
                cout << "\nSummary for " << search_date << ":\n";
                cout << "Total Sales: " << found_count << "\n";
                cout << "Total Revenue: " << fixed << setprecision(2) << date_total << " BDT\n";
                setColor(7);
            }
            break;
        }
        case 4: {
            map<string, int> daily_counts;
            map<string, double> daily_totals;
            
            for(const auto& record : sales_records) {
                string date = record.substr(0, 10);
                stringstream ss(record);
                string temp_date, customer, amount;
                getline(ss, temp_date, '|');
                getline(ss, customer, '|');
                getline(ss, amount, '|');
                
                daily_counts[date]++;
                daily_totals[date] += stod(amount);
            }
            
            clearScreen();
            setColor(11);
            cout << "\n=== SALES SUMMARY ===\n";
            cout << "┌────────────┬─────────┬─────────────┬─────────────┐\n";
            cout << "│ Date       │ Sales   │ Revenue     │ Avg Sale    │\n";
            cout << "├────────────┼─────────┼─────────────┼─────────────┤\n";
            setColor(7);
            
            for(const auto& day : daily_counts) {
                double avg = daily_totals[day.first] / day.second;
                cout << "│ " << left << setw(10) << day.first
                     << " │ " << right << setw(7) << day.second
                     << " │ " << setw(11) << fixed << setprecision(2) << daily_totals[day.first]
                     << " │ " << setw(11) << avg << " │\n";
            }
            
            setColor(11);
            cout << "└────────────┴─────────┴─────────────┴─────────────┘\n";
            setColor(7);
            break;
        }
        default:
            setColor(4);
            cout << "Invalid choice!\n";
            setColor(7);
            sleepMs(1500);
            return;
    }
    
    pauseSystem();
}

// Add this function before main()

void generate_daily_report() {
    clearScreen();
    setColor(11);
    cout << "\n=== DAILY REPORT GENERATOR ===\n";
    setColor(7);
    
    auto inv = load_inventory();
    auto customers = load_customers();
    
    string current_date = get_current_datetime().substr(0, 10);
    string filename = REPORT_FOLDER + "/DailyReport_" + current_date + ".txt";
    
    ofstream report(filename);
    if(!report.is_open()) {
        setColor(4);
        cout << "❌ Error creating report file!\n";
        setColor(7);
        sleepMs(2000);
        return;
    }
    
    // Generate progress bar
    cout << "Generating report";
    for(int i = 0; i <= 100; i += 10) {
        show_progress_bar(i);
        sleepMs(100);
    }
    cout << "\n\n";
    
    // Report Header
    report << "===============================================\n";
    report << "           DAILY BUSINESS REPORT\n";
    report << "===============================================\n";
    report << "Date: " << get_current_datetime() << "\n";
    report << "Report Generated: " << get_current_datetime() << "\n\n";
    
    // Inventory Summary
    report << "INVENTORY SUMMARY:\n";
    report << "==================\n";
    report << "Total Items: " << inv.size() << "\n";
    
    int low_stock_count = 0, out_of_stock_count = 0;
    double total_inventory_value = 0;
    
    for(const auto& item : inv) {
        total_inventory_value += item.rate * item.qty;
        if(item.qty == 0) {
            out_of_stock_count++;
        } else if(item.qty <= item.low_stock_threshold) {
            low_stock_count++;
        }
    }
    
    report << "Total Inventory Value: " << fixed << setprecision(2) << total_inventory_value << " BDT\n";
    report << "Low Stock Items: " << low_stock_count << "\n";
    report << "Out of Stock Items: " << out_of_stock_count << "\n\n";
    
    // Low Stock Items Details
    if(low_stock_count > 0) {
        report << "LOW STOCK ITEMS:\n";
        report << "=================\n";
        for(const auto& item : inv) {
            if(item.qty <= item.low_stock_threshold && item.qty > 0) {
                report << "- " << item.name << " (Stock: " << item.qty 
                       << ", Threshold: " << item.low_stock_threshold << ")\n";
            }
        }
        report << "\n";
    }
    
    // Out of Stock Items
    if(out_of_stock_count > 0) {
        report << "OUT OF STOCK ITEMS:\n";
        report << "====================\n";
        for(const auto& item : inv) {
            if(item.qty == 0) {
                report << "- " << item.name << " (Rate: " << item.rate << " BDT)\n";
            }
        }
        report << "\n";
    }
    
    // Customer Summary
    report << "CUSTOMER SUMMARY:\n";
    report << "=================\n";
    report << "Total Customers: " << customers.size() << "\n";
    
    double total_customer_value = 0;
    int active_customers = 0;
    time_t current_time = time(0);
    time_t one_month_ago = current_time - (30 * 24 * 60 * 60); // 30 days ago
    
    for(const auto& customer : customers) {
        total_customer_value += customer.total_spent;
        if(customer.last_visit > one_month_ago) {
            active_customers++;
        }
    }
    
    report << "Total Customer Spending: " << fixed << setprecision(2) << total_customer_value << " BDT\n";
    report << "Active Customers (Last 30 days): " << active_customers << "\n\n";
    
    // Top Customers
    if(!customers.empty()) {
        report << "TOP 5 CUSTOMERS BY SPENDING:\n";
        report << "============================\n";
        
        auto sorted_customers = customers;
        sort(sorted_customers.begin(), sorted_customers.end(), 
             [](const Customer& a, const Customer& b) {
                 return a.total_spent > b.total_spent;
             });
        
        for(size_t i = 0; i < min(size_t(5), sorted_customers.size()); i++) {
            report << (i+1) << ". " << sorted_customers[i].name 
                   << " - " << sorted_customers[i].total_spent << " BDT"
                   << " (" << sorted_customers[i].visit_count << " visits)\n";
        }
        report << "\n";
    }
    
    // Category Analysis
    map<string, int> category_count;
    map<string, double> category_value;
    
    for(const auto& item : inv) {
        string category = item.category.empty() ? "Uncategorized" : item.category;
        category_count[category]++;
        category_value[category] += item.rate * item.qty;
    }
    
    if(!category_count.empty()) {
        report << "CATEGORY ANALYSIS:\n";
        report << "==================\n";
        for(const auto& cat : category_count) {
            report << cat.first << ": " << cat.second << " items, Value: " 
                   << fixed << setprecision(2) << category_value[cat.first] << " BDT\n";
        }
        report << "\n";
    }
    
    // Recommendations
    report << "RECOMMENDATIONS:\n";
    report << "================\n";
    if(low_stock_count > 0) {
        report << "- Restock " << low_stock_count << " low stock items\n";
    }
    if(out_of_stock_count > 0) {
        report << "- Urgently restock " << out_of_stock_count << " out of stock items\n";
    }
    if(active_customers < customers.size() * 0.5) {
        report << "- Consider customer retention strategies\n";
    }
    
    report << "\n";
    report << "===============================================\n";
    report << "         END OF DAILY REPORT\n";
    report << "===============================================\n";
    
    report.close();
    
    setColor(10);
    cout << "✅ Daily report generated successfully!\n";
    cout << "📁 File saved as: " << filename << "\n";
    setColor(7);
    
    cout << "\nWould you like to view the report summary? (y/n): ";
    char view;
    cin >> view;
    wait_and_flush();
    
    if(view == 'y' || view == 'Y') {
        clearScreen();
        setColor(11);
        cout << "\n=== REPORT SUMMARY ===\n";
        setColor(7);
        cout << "📦 Total Items: " << inv.size() << "\n";
        cout << "💰 Inventory Value: " << fixed << setprecision(2) << total_inventory_value << " BDT\n";
        cout << "👥 Total Customers: " << customers.size() << "\n";
        cout << "⚠️  Low Stock: " << low_stock_count << " items\n";
        cout << "❌ Out of Stock: " << out_of_stock_count << " items\n";
        cout << "💸 Customer Spending: " << total_customer_value << " BDT\n";
        pauseSystem();
    }
}

// Enhanced Main Menu with new features
int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif
    
    ensure_directories();
    
    bool exit = false;
    while(!exit) {
        clearScreen(); 
        
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
        
        cout << "\n\t\t\t    Enhanced Supermarket POS & Billing System v2.0\n";
        cout << "\t\t\t    ================================================\n\n";
        
        setColor(11);
        cout << "\t┌────────────────────────────────────────────────────────────┐\n";
        cout << "\t│                        MAIN MENU                           │\n";
        cout << "\t├────────────────────────────────────────────────────────────┤\n";
        cout << "\t│  1. 📦 Inventory Management                                │\n";
        cout << "\t│  2. 💰 Create Bill / Process Sale                          │\n";
        cout << "\t│  3. 📋 Display Full Inventory                              │\n";
        cout << "\t│  4. 🔍 Search Inventory                                    │\n";
        cout << "\t│  5. 🗑️ Delete Item from Inventory                          │\n";
        cout << "\t│  6. 📊 Generate Daily Report                               │\n";
        cout << "\t│  7. 📈 View Sales History                                  │\n";
        cout << "\t│  8. 👥 Customer Management                                 │\n";
        cout << "\t│  9. 📱 Barcode Scanner                                     │\n";
        cout << "\t│ 10. ⚡ Quick Sale Mode                                     │\n";
        cout << "\t│ 11. 📊 Dashboard                                           │\n";
        cout << "\t│ 12. 🛒 Generate Purchase Order                             │\n";
        cout << "\t│ 13. 💾 Export to CSV                                       │\n";
        cout << "\t│ 14. ❌ Exit System                                         │\n";
        cout << "\t└────────────────────────────────────────────────────────────┘\n";
        setColor(7);
        
        cout << "\n\tEnter your choice (1-14): ";
        
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
            case 1: add_item_flow(); break;
            case 2: print_bill_flow(); break;
            case 3: display_inventory(); break;
            case 4: search_inventory(); break;
            case 5: delete_item_flow(); break;
            case 6: generate_daily_report(); break;
            case 7: view_sales_history(); break;
            case 8: customer_management_menu(); break;
            case 9: scan_barcode_mode(); break;
            case 10: quick_sale_mode(); break;
            case 11: show_dashboard(); break;
            case 12: generate_purchase_order(); break;
            case 13: export_to_csv(); break;
            case 14: 
                setColor(14);
                cout << "\n\tThank you for using Enhanced POS System! Goodbye!\n";
                setColor(7);
                sleepMs(2000);
                exit = true; 
                break;
            default: 
                setColor(4); 
                cout << "\tInvalid choice! Please select 1-14.\n"; 
                setColor(7); 
                sleepMs(1500);
        }
    }
    return 0;
}