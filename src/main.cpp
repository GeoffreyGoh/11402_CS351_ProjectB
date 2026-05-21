#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm> // For std::remove_if
#include <cctype>    // For std::isspace
#include <fstream>
#include <optional> // For std::optional
#include <variant>  // For std::variant
#include <regex>    // For std::regex

// --- Utils Namespace ---
namespace Utils {
    // Splits a string by a delimiter
    std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // Trims leading/trailing whitespace from a string
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r\f\v");
        if (std::string::npos == first) return str;
        size_t last = str.find_last_not_of(" \t\n\r\f\v");
        return str.substr(first, (last - first + 1));
    }
} // namespace Utils

// --- StorageManager Declarations and Definitions ---
// A row is a map from column name to its string value
using Row = std::map<std::string, std::string>;
// A table is a vector of rows
using Table = std::vector<Row>;

class StorageManager {
public:
    bool loadTable(const std::string& table_name, const std::string& file_path);
    std::optional<const Table*> getTable(const std::string& table_name) const;

private:
    std::map<std::string, Table> tables_;
};

bool StorageManager::loadTable(const std::string& table_name, const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << file_path << std::endl;
        return false;
    }

    std::string line;
    if (!std::getline(file, line)) {
        std::cerr << "Error: CSV file is empty or cannot read header." << std::endl;
        return false;
    }

    // Parse header
    std::vector<std::string> headers = Utils::split(line, ',');
    for (std::string& header : headers) {
        header = Utils::trim(header);
    }

    Table new_table;
    while (std::getline(file, line)) {
        std::vector<std::string> values = Utils::split(line, ',');
        Row row;
        for (size_t i = 0; i < headers.size(); ++i) {
            if (i < values.size()) {
                row[headers[i]] = Utils::trim(values[i]);
            } else {
                row[headers[i]] = ""; // Handle missing values
            }
        }
        new_table.push_back(row);
    }

    tables_[table_name] = new_table;
    return true;
}

std::optional<const Table*> StorageManager::getTable(const std::string& table_name) const {
    auto it = tables_.find(table_name);
    if (it != tables_.end()) {
        return &it->second;
    }
    return std::nullopt;
}

// --- QueryParser Declarations and Definitions ---
// Structs to represent parsed query plans
struct LoadPlan {
    std::string table_name;
    std::string file_path;
};

struct SelectPlan {
    std::vector<std::string> columns;
    std::string table_name;
    std::string where_column;
    std::string where_value;
    bool has_where = false;
};

// A variant to hold any type of query plan, or std::monostate for an invalid plan
using QueryPlan = std::variant<LoadPlan, SelectPlan, std::monostate>;

class QueryParser {
public:
    static QueryPlan parse(const std::string& query);

private:
    // Helper to remove quotes from a string
    static std::string removeQuotes(const std::string& s);
};

std::string QueryParser::removeQuotes(const std::string& s) {
    if (s.length() >= 2 && ((s.front() == '\'' && s.back() == '\'') || (s.front() == '"' && s.back() == '"'))) {
        return s.substr(1, s.length() - 2);
    }
    return s;
}

QueryPlan QueryParser::parse(const std::string& query) {
    // LOAD table FROM 'file'
    std::regex load_pattern(R"(LOAD\s+(\w+)\s+FROM\s+'([^']+)')", std::regex::icase);
    std::smatch load_match;
    if (std::regex_match(query, load_match, load_pattern)) {
        LoadPlan plan;
        plan.table_name = load_match[1].str();
        plan.file_path = load_match[2].str();
        return plan;
    }

    // SELECT columns FROM table [WHERE column = value]
    std::regex select_pattern(R"(SELECT\s+(.+)\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?)", std::regex::icase);
    std::smatch select_match;
    if (std::regex_match(query, select_match, select_pattern)) {
        SelectPlan plan;
        
        // Columns
        std::string cols_str = select_match[1].str();
        std::vector<std::string> raw_cols = Utils::split(cols_str, ',');
        for (const std::string& col : raw_cols) {
            plan.columns.push_back(Utils::trim(col));
        }

        // Table name
        plan.table_name = select_match[2].str();

        // WHERE clause
        if (select_match[3].matched) {
            plan.has_where = true;
            std::string where_clause = select_match[3].str();
            std::regex where_pattern(R"((.+)\s*=\s*(.+))");
            std::smatch where_match;
            if (std::regex_match(where_clause, where_match, where_pattern)) {
                plan.where_column = Utils::trim(where_match[1].str());
                plan.where_value = removeQuotes(Utils::trim(where_match[2].str()));
            } else {
                // Malformed WHERE clause
                std::cerr << "Warning: Malformed WHERE clause: " << where_clause << std::endl;
                return std::monostate{};
            }
        }
        return plan;
    }

    // If no pattern matches
    return std::monostate{};
}

// --- QueryEngine Declarations and Definitions ---
// Define a type for query results (e.g., a vector of rows)
using QueryResult = std::variant<std::string, Table>;

class QueryEngine {
public:
    explicit QueryEngine(StorageManager& storage_manager);
    QueryResult execute(const QueryPlan& plan);
private:
    StorageManager& storage_;
};

QueryEngine::QueryEngine(StorageManager& storage_manager) : storage_(storage_manager) {}

QueryResult QueryEngine::execute(const QueryPlan& plan) {
    if (std::holds_alternative<std::monostate>(plan)) {
        return "Error: Unsupported or malformed query.";
    }

    if (const auto* load_plan = std::get_if<LoadPlan>(&plan)) {
        bool success = storage_.loadTable(load_plan->table_name, load_plan->file_path);
        return success ? "Table '" + load_plan->table_name + "' loaded." : "Error: File not found or could not be loaded.";
    }

    if (const auto* select_plan = std::get_if<SelectPlan>(&plan)) {
        auto table_opt = storage_.getTable(select_plan->table_name);
        if (!table_opt) {
            return "Error: Table '" + select_plan->table_name + "' not found. Load it first.";
        }
        const Table* data = table_opt.value();

        Table filtered_results;
        // Filtering (WHERE clause)
        if (select_plan->has_where) {
            for (const auto& row : *data) {
                auto it = row.find(select_plan->where_column);
                if (it != row.end() && it->second == select_plan->where_value) {
                    filtered_results.push_back(row);
                }
            }
        } else {
            filtered_results = *data; // No WHERE clause, all rows
        }

        // Projection (Column selection)
        Table projected_results;
        if (!filtered_results.empty()) {
            bool select_all = false;
            if (select_plan->columns.size() == 1 && select_plan->columns[0] == "*") {
                select_all = true;
            }

            for (const auto& row : filtered_results) {
                Row projected_row;
                if (select_all) {
                    projected_row = row;
                } else {
                    for (const auto& col_name : select_plan->columns) {
                        auto it = row.find(col_name);
                        if (it != row.end()) {
                            projected_row[col_name] = it->second;
                        }
                    }
                }
                projected_results.push_back(projected_row);
            }
        }
        return projected_results;
    }
    return "Unknown command.";
}

// --- Main Function ---
// Helper function to print a row
void printRow(const Row& row) {
    bool first = true;
    std::cout << "{";
    for (const auto& pair : row) {
        if (!first) {
            std::cout << ", ";
        }
        std::cout << "'" << pair.first << "': '" << pair.second << "'";
        first = false;
    }
    std::cout << "}" << std::endl;
}

#ifndef TEST_MODE
int main() {
    StorageManager storage;
    QueryEngine engine(storage);

    std::cout << "CSV Mini Database CLI (C++). Type 'exit' to quit." << std::endl;
    std::string query;

    while (true) {
        std::cout << "db> ";
        std::getline(std::cin, query);

        if (query.empty()) continue;
        if (query == "exit") break;

        try {
            QueryPlan plan = QueryParser::parse(query);
            QueryResult result = engine.execute(plan);

            if (const auto* msg = std::get_if<std::string>(&result)) {
                std::cout << *msg << std::endl;
            } else if (const auto* table_result = std::get_if<Table>(&result)) {
                for (const auto& row : *table_result) {
                    printRow(row);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Runtime Error: " << e.what() << std::endl;
        }
    }
    return 0;
}
#endif