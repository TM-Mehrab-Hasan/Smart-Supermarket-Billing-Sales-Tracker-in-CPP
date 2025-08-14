#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <windows.h>

using namespace std;

// ---------- Windows Console Color Utility ----------
void setColor(int color) {
    // 0 = Black, 1 = Blue, 2 = Green, 4 = Red, 7 = White (default)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void wait_and_flush() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

struct ItemRec {
    string name;
    int rate = 0;
    int qty = 0;
};

static const string INVENTORY_FILE = "Bill.txt";

// ---------- Inventory Handling ----------
bool parse_line(const string& line, ItemRec& out) {
    size_t p1 = line.find('|');
    if (p1 == string::npos) return false;
    size_t p2 = line.find('|', p1 + 1);
    if (p2 == string::npos) return false;

    out.name = line.substr(0, p1);
    try {
        out.rate = stoi(line.substr(p1 + 1, p2 - p1 - 1));
        out.qty  = stoi(line.substr(p2 + 1));
    } catch (...) { return false; }

    return (out.rate >= 0 && out.qty >= 0 && !out.name.empty());
}

string serialize_line(const ItemRec& r) {
    return r.name + "|" + to_string(r.rate) + "|" + to_string(r.qty);
}

vector<ItemRec> load_inventory() {
    vector<ItemRec> inv;
    ifstream in(INVENTORY_FILE);
    if (!in.is_open()) return inv;

    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        ItemRec r;
        if (parse_line(line, r)) inv.push_back(r);
    }
    return inv;
}

bool save_inventory(const vector<ItemRec>& inv) {
    ofstream out(INVENTORY_FILE, ios::trunc);
    if (!out.is_open()) return false;
    for (const auto& r : inv) out << serialize_line(r) << "\n";
    return true;
}

int find_item(const vector<ItemRec>& inv, const string& name) {
    for (size_t i = 0; i < inv.size(); ++i)
        if (inv[i].name == name) return static_cast<int>(i);
    return -1;
}

// ---------- Features ----------
void add_item_flow() {
    bool close = false;
    while (!close) {
        system("cls");
        setColor(11); // Light Cyan
        cout << "\t1. Add/Update Item\n\t2. Close\n\tEnter Choice: ";
        setColor(7); // White

        int choice;
        if (!(cin >> choice)) { wait_and_flush(); continue; }
        wait_and_flush();

        if (choice == 1) {
            string name;
            int rate, quant;

            setColor(14); // Yellow
            cout << "\tEnter Item Name: ";
            setColor(7); cin.ignore(); getline(cin, name);

            setColor(14); cout << "\tEnter Rate Of Item (BDT): ";
            setColor(7);
            if (!(cin >> rate)) { cout << "\tInvalid rate.\n"; wait_and_flush(); Sleep(1200); continue; }

            setColor(14); cout << "\tEnter Quantity Of Item: ";
            setColor(7);
            if (!(cin >> quant)) { cout << "\tInvalid quantity.\n"; wait_and_flush(); Sleep(1200); continue; }
            wait_and_flush();

            if (rate < 0 || quant < 0 || name.empty()) {
                setColor(4); // Red
                cout << "\tInvalid data.\n";
                setColor(7);
                Sleep(1500);
                continue;
            }

            auto inv = load_inventory();
            int idx = find_item(inv, name);
            if (idx >= 0) {
                inv[idx].rate = rate;
                inv[idx].qty += quant;
            } else {
                inv.push_back(ItemRec{name, rate, quant});
            }

            if (!save_inventory(inv)) {
                setColor(4); cout << "\tError: cannot write " << INVENTORY_FILE << "\n"; setColor(7);
            } else {
                setColor(10); cout << "\tItem Added/Updated Successfully\n"; setColor(7);
            }
            Sleep(1500);
        } 
        else if (choice == 2) {
            setColor(11); cout << "\tBack To Main Menu!\n"; setColor(7);
            Sleep(900);
            close = true;
        } 
        else {
            setColor(4); cout << "\tInvalid choice.\n"; setColor(7);
            Sleep(900);
        }
    }
}

void print_bill_flow() {
    system("cls");
    long long total = 0;
    vector<pair<ItemRec, int>> receipt;

    bool close = false;
    while (!close) {
        system("cls");
        setColor(11);
        cout << "\t1. Add Bill Item\n\t2. Close Session\n\tEnter Choice: ";
        setColor(7);
        int choice;
        if (!(cin >> choice)) { wait_and_flush(); continue; }
        wait_and_flush();

        if (choice == 1) {
            string name; int quant;
            setColor(14); cout << "\tEnter Item: "; setColor(7); cin.ignore(); getline(cin, name);
            setColor(14); cout << "\tEnter Quantity: "; setColor(7);
            if (!(cin >> quant)) { cout << "\tInvalid quantity.\n"; wait_and_flush(); Sleep(1200); continue; }
            wait_and_flush();

            if (quant <= 0) {
                setColor(4); cout << "\tQuantity must be positive.\n"; setColor(7);
                Sleep(1200);
                continue;
            }

            auto inv = load_inventory();
            int idx = find_item(inv, name);
            if (idx < 0) {
                setColor(4); cout << "\tItem Not Available!\n"; setColor(7);
                Sleep(1500);
                continue;
            }

            if (inv[idx].qty < quant) {
                setColor(4); cout << "\tSorry, only " << inv[idx].qty << " left for \"" << inv[idx].name << "\".\n"; setColor(7);
                Sleep(1500);
                continue;
            }

            inv[idx].qty -= quant;
            if (!save_inventory(inv)) {
                setColor(4); cout << "\tError: could not update inventory.\n"; setColor(7);
                Sleep(1500);
                continue;
            }

            long long amount = 1LL * inv[idx].rate * quant;
            total += amount;
            receipt.push_back({inv[idx], quant});

            setColor(10); // Green
            cout << "\n\tItem\tRate(BDT)\tQuantity\tAmount(BDT)\n";
            for (auto& p : receipt) {
                cout << "\t" << p.first.name << "\t" << p.first.rate << "\t\t" << p.second << "\t\t" << (1LL*p.first.rate*p.second) << "\n";
            }
            setColor(7);
            Sleep(2000);
        } 
        else if (choice == 2) {
            setColor(11); cout << "\tCounting Total Bill...\n"; setColor(7);
            Sleep(1200);
            close = true;
        } 
        else {
            setColor(4); cout << "\tInvalid choice.\n"; setColor(7);
            Sleep(900);
        }
    }

    system("cls");
    setColor(13); // Magenta
    cout << "\n\t===== FINAL RECEIPT =====\n";
    cout << "\tItem\tRate\tQty\tAmount\n";
    for (auto& p : receipt) {
        const auto& it = p.first; int q = p.second;
        cout << "\t" << it.name << "\t" << it.rate << "\t" << q << "\t" << (1LL*it.rate*q) << "\n";
    }
    setColor(14);
    cout << "\n\tTotal Bill ----------------- : " << total << " BDT\n\n";
    setColor(10); cout << "\tThanks For Shopping!\n"; setColor(7);
    Sleep(2500);
}

int main() {
    bool exit = false;
    while (!exit) {
        system("cls");
        setColor(9); // Blue
        cout << "\tWelcome To Super Market Billing System\n";
        cout << "\t**************************************\n";
        setColor(11);
        cout << "\t\t1. Add/Update Item\n";
        cout << "\t\t2. Print Bill (Sell)\n";
        cout << "\t\t3. Exit\n";
        setColor(7);
        cout << "\t\tEnter Choice: ";

        int val;
        if (!(cin >> val)) { wait_and_flush(); continue; }
        wait_and_flush();

        if (val == 1) add_item_flow();
        else if (val == 2) print_bill_flow();
        else if (val == 3) {
            setColor(12); cout << "\tGood Luck!\n"; setColor(7);
            Sleep(1000);
            exit = true;
        }
        else {
            setColor(4); cout << "\tInvalid choice.\n"; setColor(7);
            Sleep(900);
        }
    }
    return 0;
}
