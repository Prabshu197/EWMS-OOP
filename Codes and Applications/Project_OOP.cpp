#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <stdexcept>
#include <iomanip>
using namespace std;

// ---------- Abstract base class ----------
class Waste {
protected:
    int id;
    string name;
    float weight;                 // in kg
    static int totalItems;        // shared by all objects

public:
    Waste(int id, const string& name, float weight) : id(id), name(name), weight(weight) {
        if (id <= 0) throw invalid_argument("ID must be a positive number.");
        if (weight <= 0) throw invalid_argument("Weight must be greater than 0.");
        if (name.empty()) throw invalid_argument("Name cannot be empty.");
        totalItems++;
    }
    virtual ~Waste() { totalItems--; }

    // getters (data stays hidden)
    int getId() const { return id; }
    const string& getName() const { return name; }
    float getWeight() const { return weight; }
    static int getTotalItems() { return totalItems; }

    // pure virtual functions -> every derived class must override them
    virtual string getType() const = 0;
    virtual void recyclingInfo() const = 0;
    virtual int pointsPerKg() const = 0;

    int rewardPoints() const { return static_cast<int>(weight * pointsPerKg()); }

    virtual void display() const {
        cout << "ID: " << id << " | " << left << setw(14) << name
             << " | " << setw(9) << getType()
             << " | " << fixed << setprecision(2) << weight << " kg"
             << " | " << rewardPoints() << " pts\n";
    }

    // used for saving to file
    string toRecord() const {
        return getType() + "|" + to_string(id) + "|" + name + "|" + to_string(weight);
    }

    // operator overloading: compare two items by weight
    bool operator>(const Waste& other) const { return weight > other.weight; }
};
int Waste::totalItems = 0;

// ---------- Derived classes (inheritance) ----------
class Mobile : public Waste {
public:
    Mobile(int id, const string& n, float w) : Waste(id, n, w) {}
    string getType() const override { return "Mobile"; }
    int pointsPerKg() const override { return 100; }
    void recyclingInfo() const override {
        cout << "[Mobile] Remove SIM and memory card, wipe personal data,\n"
             << "         remove the battery and hand it to an authorized recycler.\n";
    }
};

class Computer : public Waste {
public:
    Computer(int id, const string& n, float w) : Waste(id, n, w) {}
    string getType() const override { return "Computer"; }
    int pointsPerKg() const override { return 60; }
    void recyclingInfo() const override {
        cout << "[Computer] Erase the hard disk, donate if still working,\n"
             << "           otherwise send to an authorized e-waste centre.\n";
    }
};

class Battery : public Waste {
public:
    Battery(int id, const string& n, float w) : Waste(id, n, w) {}
    string getType() const override { return "Battery"; }
    int pointsPerKg() const override { return 150; }
    void recyclingInfo() const override {
        cout << "[Battery] Hazardous! Never throw in regular waste.\n"
             << "          Use a designated battery collection box.\n";
    }
};

// ---------- Manager class (composition) ----------
class EWasteManager {
    vector<unique_ptr<Waste>> items;     // polymorphic collection
    const string fileName = "ewaste_data.txt";

    static unique_ptr<Waste> create(const string& type, int id, const string& name, float w) {
        if (type == "Mobile")   return make_unique<Mobile>(id, name, w);
        if (type == "Computer") return make_unique<Computer>(id, name, w);
        if (type == "Battery")  return make_unique<Battery>(id, name, w);
        throw invalid_argument("Unknown waste type: " + type);
    }
    bool idExists(int id) const {
        for (const auto& i : items) if (i->getId() == id) return true;
        return false;
    }

public:
    void add(const string& type, int id, const string& name, float w) {
        if (idExists(id)) throw invalid_argument("This ID already exists.");
        items.push_back(create(type, id, name, w));
    }

    void displayAll() const {
        if (items.empty()) { cout << "\nNo e-waste records yet.\n"; return; }
        cout << "\n--- E-Waste Records ---\n";
        for (const auto& i : items) i->display();   // runtime polymorphism
    }

    void recyclingFor(int id) const {
        for (const auto& i : items)
            if (i->getId() == id) { i->recyclingInfo(); return; }
        cout << "No item found with ID " << id << ".\n";
    }

    void summary() const {
        float totalWeight = 0; int points = 0;
        const Waste* heaviest = nullptr;
        for (const auto& i : items) {
            totalWeight += i->getWeight();
            points += i->rewardPoints();
            if (!heaviest || *i > *heaviest) heaviest = i.get();   // overloaded >
        }
        cout << "\n--- Summary ---\n"
             << "Items recorded : " << Waste::getTotalItems() << "\n"
             << "Total weight   : " << fixed << setprecision(2) << totalWeight << " kg\n"
             << "Reward points  : " << points << "\n";
        if (heaviest) cout << "Heaviest item  : " << heaviest->getName() << "\n";
    }

    void save() const {
        ofstream out(fileName);
        if (!out) throw runtime_error("Could not open file for saving.");
        for (const auto& i : items) out << i->toRecord() << "\n";
        cout << "Saved " << items.size() << " record(s) to " << fileName << ".\n";
    }

    void load() {
        ifstream in(fileName);
        if (!in) return;                     // first run: no file yet
        string line;
        while (getline(in, line)) {
            stringstream ss(line);
            string type, sid, name, sw;
            if (getline(ss, type, '|') && getline(ss, sid, '|') &&
                getline(ss, name, '|') && getline(ss, sw)) {
                try { add(type, stoi(sid), name, stof(sw)); } catch (...) {}
            }
        }
    }
};

// ---------- Input helpers ----------
int readInt(const string& prompt) {
    int v;
    cout << prompt;
    while (!(cin >> v)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid number, try again: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return v;
}
float readFloat(const string& prompt) {
    float v;
    cout << prompt;
    while (!(cin >> v)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid number, try again: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return v;
}

int main() {
    EWasteManager manager;
    manager.load();
    int choice;

    do {
        cout << "\n===== E-WASTE MANAGEMENT SYSTEM (EcoLoop) =====\n"
             << "1. Add E-Waste\n"
             << "2. Display All E-Waste\n"
             << "3. Recycling Information for an Item\n"
             << "4. Summary and Reward Points\n"
             << "5. Save to File\n"
             << "6. Exit\n";
        choice = readInt("Enter your choice: ");

        try {
            switch (choice) {
            case 1: {
                cout << "Type: 1. Mobile  2. Computer  3. Battery\n";
                int t = readInt("Select type: ");
                string type = (t == 1) ? "Mobile" : (t == 2) ? "Computer" : (t == 3) ? "Battery" : "";
                if (type.empty()) throw invalid_argument("Invalid type selected.");
                int id = readInt("Enter Waste ID: ");
                string name;
                cout << "Enter Waste Name: ";
                getline(cin, name);
                float w = readFloat("Enter Weight in kg: ");
                manager.add(type, id, name, w);
                cout << "\nE-Waste added successfully!\n";
                break;
            }
            case 2: manager.displayAll(); break;
            case 3: manager.recyclingFor(readInt("Enter Waste ID: ")); break;
            case 4: manager.summary(); break;
            case 5: manager.save(); break;
            case 6: manager.save(); cout << "\nThank you for using the system!\n"; break;
            default: cout << "\nInvalid choice! Try again.\n";
            }
        } catch (const exception& e) {
            cout << "\nError: " << e.what() << "\n";
        }
    } while (choice != 6);

    return 0;
}
