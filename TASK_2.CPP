#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <memory>
#include <thread>
#include <mutex>
#include <iomanip>

using namespace std;


class ResultSystemException : public exception {
protected:
    string message;
public:
    explicit ResultSystemException(const string& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class InvalidMarksException : public ResultSystemException {
public:
    explicit InvalidMarksException(int invalidMark)
        : ResultSystemException("Invalid Marks Error: Mark (" + to_string(invalidMark) +
                                ") must be strictly between 0 and 100.") {}
};

class FileAccessException : public ResultSystemException {
public:
    explicit FileAccessException(const string& filename)
        : ResultSystemException("File I/O Error: Unable to open or access '" + filename + "'.") {}
};

class AuditLogger {
private:
    mutex logMutex;
    AuditLogger() {} 

public:
    AuditLogger(const AuditLogger&) = delete;
    void operator=(const AuditLogger&) = delete;

    static AuditLogger& getInstance() {
        static AuditLogger instance; /
        return instance;
    }

    void log(const string& event) {
        lock_guard<mutex> lock(logMutex); // RAII mutex lock
        ofstream logFile("audit_log.txt", ios::app);
        if (logFile) {
            logFile << "[AUDIT] " << event << "\n";
        }
    }
};


template <typename T>
T calculateAverage(const vector<T>& values) {
    if (values.empty()) return T(0);
    T sum = accumulate(values.begin(), values.end(), T(0));
    return sum / static_cast<T>(values.size());
}

template <typename T>
char assignLetterGrade(T percentage) {
    if (percentage >= 90) return 'S';
    if (percentage >= 80) return 'A';
    if (percentage >= 70) return 'B';
    if (percentage >= 60) return 'C';
    if (percentage >= 50) return 'D';
    return 'F';
}

class Evaluable {
public:
    virtual void computeResult() = 0;            
    virtual void displayRecord() const = 0;      
    virtual string serialize() const = 0;        
    virtual ~Evaluable() {}                      
};

class Student {
protected:
    string usn;
    string name;
    string branch;

public:
    Student(string u = "", string n = "", string b = "ECE")
        : usn(move(u)), name(move(n)), branch(move(b)) {}

    virtual ~Student() {}

    string getUSN() const { return usn; }
    string getName() const { return name; }
    string getBranch() const { return branch; }
};


class Result : public Student, public Evaluable {
private:
    vector<int> marks; 
    int totalMarks;
    double percentage;
    char grade;
    bool passed;

public:
    Result(const string& u, const string& n, const string& b, const vector<int>& m)
        : Student(u, n, b), totalMarks(0), percentage(0.0), grade('F'), passed(false) {
        setMarks(m);
        computeResult();
    }

    void setMarks(const vector<int>& m) {
        for (int score : m) {
            if (score < 0 || score > 100) {
                throw InvalidMarksException(score);
            }
        }
        marks = m;
    }

    
    void computeResult() override {
        totalMarks = accumulate(marks.begin(), marks.end(), 0); 
        percentage = marks.empty() ? 0.0 : static_cast<double>(totalMarks) / marks.size();
        grade = assignLetterGrade(percentage);

        
        bool allSubjectsPassed = all_of(marks.begin(), marks.end(), [](int s) { return s >= 35; });
        passed = (percentage >= 40.0) && allSubjectsPassed;
    }

    double getPercentage() const { return percentage; }
    int getTotalMarks() const { return totalMarks; }
    char getGrade() const { return grade; }
    bool isPassed() const { return passed; }

    // Week 8: Runtime polymorphic display
    void displayRecord() const override {
        cout << left << setw(12) << usn
             << setw(18) << name
             << setw(8) << branch;
        for (int m : marks) {
            cout << setw(5) << m;
        }
        cout << setw(8) << totalMarks
             << setw(9) << fixed << setprecision(2) << percentage
             << setw(7) << grade
             << (passed ? "PASS" : "FAIL") << "\n";
    }

    
    string serialize() const override {
        ostringstream oss;
        oss << usn << "," << name << "," << branch;
        for (int m : marks) {
            oss << "," << m;
        }
        return oss.str();
    }
};

class ResultFactory {
public:
    static unique_ptr<Result> createResult(const string& usn, const string& name,
                                           const string& branch, const vector<int>& marks) {
        return make_unique<Result>(usn, name, branch, marks);
    }
};


class ResultManager {
private:
    vector<unique_ptr<Result>> records; // Week 11 & 13: vector of unique_ptr
    const string dataFile = "student_results.csv";

public:
    void addStudentRecord(const string& usn, const string& name,
                          const string& branch, const vector<int>& marks) {
        auto rec = ResultFactory::createResult(usn, name, branch, marks);
        AuditLogger::getInstance().log("Added record for USN: " + usn);
        records.push_back(move(rec));
    }

    void saveToFile() const {
        ofstream outFile(dataFile);
        if (!outFile) {
            throw FileAccessException(dataFile);
        }
        for (const auto& rec : records) {
            outFile << rec->serialize() << "\n";
        }
        outFile.close();
        AuditLogger::getInstance().log("Saved " + to_string(records.size()) + " records to " + dataFile);
        cout << "[INFO] Successfully saved " << records.size() << " records to '" << dataFile << "'.\n";
    }

    
    void loadFromFile() {
        ifstream inFile(dataFile);
        if (!inFile) {
            throw FileAccessException(dataFile);
        }
        records.clear();
        string line;
        while (getline(inFile, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string usn, name, branch, token;
            getline(ss, usn, ',');
            getline(ss, name, ',');
            getline(ss, branch, ',');

            vector<int> marks;
            while (getline(ss, token, ',')) {
                marks.push_back(stoi(token));
            }
            records.push_back(ResultFactory::createResult(usn, name, branch, marks));
        }
        inFile.close();
        AuditLogger::getInstance().log("Loaded " + to_string(records.size()) + " records from " + dataFile);
        cout << "[INFO] Successfully loaded " << records.size() << " records from '" << dataFile << "'.\n";
    }

    /
    void displayAll() const {
        cout << "\n================================================================================\n";
        cout << left << setw(12) << "USN" << setw(18) << "Name" << setw(8) << "Branch"
             << setw(5) << "M1" << setw(5) << "M2" << setw(5) << "M3" << setw(5) << "M4"
             << setw(8) << "Total" << setw(9) << "Pct(%)" << setw(7) << "Grade" << "Status\n";
        cout << "--------------------------------------------------------------------------------\n";
        for (const auto& rec : records) {
            Evaluable* basePtr = rec.get(); 
            basePtr->displayRecord();       
        }
        cout << "================================================================================\n";
    }

    
    void sortByPercentageDesc() {
        sort(records.begin(), records.end(),
             [](const unique_ptr<Result>& a, const unique_ptr<Result>& b) {
                 return a->getPercentage() > b->getPercentage();
             });
        cout << "\n[INFO] Records sorted by Percentage (Descending).\n";
    }

    void filterAndAnalyze(double cutoffPercentage) const {
        int passCount = count_if(records.begin(), records.end(),
                                 [](const unique_ptr<Result>& r) { return r->isPassed(); });

        cout << "\n--- Analytics & Filtered Results (Cutoff >= " << cutoffPercentage << "%) ---\n";
        cout << "Total Students: " << records.size()
             << " | Passed: " << passCount
             << " | Failed: " << (records.size() - passCount) << "\n";

        vector<double> allPercentages;
        for (const auto& r : records) {
            allPercentages.push_back(r->getPercentage());
        }
        cout << "Class Average Percentage: " << fixed << setprecision(2)
             << calculateAverage(allPercentages) << "%\n\n";

        cout << "Distinction / Filtered Students (>= " << cutoffPercentage << "%):\n";
        for_each(records.begin(), records.end(), [cutoffPercentage](const unique_ptr<Result>& r) {
            if (r->getPercentage() >= cutoffPercentage) {
                cout << "  -> " << r->getUSN() << " | " << left << setw(16) << r->getName()
                     << " | " << fixed << setprecision(2) << r->getPercentage()
                     << "% (Grade: " << r->getGrade() << ")\n";
            }
        });
    }

    
    void runConcurrentAuditTask() const {
        mutex reportMutex;
        int highScorers = 0;

        auto worker = [&](size_t startIdx, size_t endIdx) {
            int localCount = 0;
            for (size_t i = startIdx; i < endIdx; ++i) {
                if (records[i]->getPercentage() >= 85.0) {
                    ++localCount;
                }
            }
            lock_guard<mutex> guard(reportMutex);
            highScorers += localCount;
        };

        size_t mid = records.size() / 2;
        thread t1(worker, 0, mid);
        thread t2(worker, mid, records.size());
        t1.join();
        t2.join();

        cout << "\n[THREAD-AUDIT] Concurrent scan completed. High scorers (>=85%): "
             << highScorers << "\n";
    }
};


int main() {
    ResultManager manager;

    cout << "=== STUDENT RESULT MANAGEMENT SYSTEM (ECE121 TASK 2: WEEKS 7-13) ===\n\n";

  
    try {
        manager.addStudentRecord("1NT23EC001", "Aarav Sharma",   "ECE", {88, 92, 85, 90});
        manager.addStudentRecord("1NT23EC002", "Diya Nair",      "ECE", {95, 94, 96, 91});
        manager.addStudentRecord("1NT23EC003", "Rohan Kulkarni", "ECE", {64, 70, 68, 72});
        manager.addStudentRecord("1NT23EC004", "Sneha Rao",      "ECE", {45, 30, 52, 48}); 
        manager.addStudentRecord("1NT23EC005", "Vikram Menon",   "ECE", {82, 79, 86, 84});
        cout << "[INFO] 5 valid student records added successfully.\n";
    } catch (const ResultSystemException& e) {
        cerr << "[EXCEPTION] " << e.what() << "\n";
    }

    
    cout << "\n--- Testing Mark Validation & Custom Exception Handling ---\n";
    try {
        cout << "Attempting to add record with invalid mark (108)...\n";
        manager.addStudentRecord("1NT23EC099", "Test Invalid", "ECE", {78, 108, 80, 75});
    } catch (const InvalidMarksException& e) {
        cerr << "[CAUGHT EXPECTED EXCEPTION] " << e.what() << "\n";
    } catch (const ResultSystemException& e) {
        cerr << "[CAUGHT BASE EXCEPTION] " << e.what() << "\n";
    }

    // 3. Saving and Reloading Records via File I/O (Week 10)
    cout << "\n--- Testing File Persistence (Save & Load CSV) ---\n";
    try {
        manager.saveToFile();
        manager.loadFromFile();
    } catch (const FileAccessException& e) {
        cerr << "[FILE EXCEPTION] " << e.what() << "\n";
    }

    
    cout << "\n--- All Loaded Student Records ---";
    manager.displayAll();

    
    manager.sortByPercentageDesc();
    manager.displayAll();
    manager.filterAndAnalyze(80.0);

    
    manager.runConcurrentAuditTask();

    return 0;
}
