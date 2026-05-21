#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <fstream>

// Define TEST_MODE to skip the main function in main.cpp when it is included
#define TEST_MODE
#include "main.cpp"

void test_utils() {
    std::cout << "Running test_utils..." << std::endl;
    
    auto tokens = Utils::split("id,name,dept", ',');
    assert(tokens.size() == 3);
    assert(tokens[0] == "id");
    assert(tokens[1] == "name");
    assert(tokens[2] == "dept");

    assert(Utils::trim("  hello  ") == "hello");
    assert(Utils::trim("\tworld\n") == "world");
    
    std::cout << "test_utils passed!" << std::endl;
}

void test_parser() {
    std::cout << "Running test_parser..." << std::endl;

    auto load_plan = QueryParser::parse("LOAD users FROM 'users.csv'");
    assert(std::holds_alternative<LoadPlan>(load_plan));
    assert(std::get<LoadPlan>(load_plan).table_name == "users");

    auto select_plan = QueryParser::parse("SELECT name FROM employees WHERE id = '1'");
    assert(std::holds_alternative<SelectPlan>(select_plan));
    auto plan = std::get<SelectPlan>(select_plan);
    assert(plan.table_name == "employees");
    assert(plan.has_where == true);
    assert(plan.where_column == "id");
    assert(plan.where_value == "1");

    std::cout << "test_parser passed!" << std::endl;
}

void test_engine_execution() {
    std::cout << "Running test_engine_execution..." << std::endl;

    // Setup temporary CSV file
    std::ofstream test_file("test_data.csv");
    test_file << "id,name,role\n1,Alice,Dev\n2,Bob,Manager";
    test_file.close();

    StorageManager storage;
    QueryEngine engine(storage);

    // Test LOAD
    auto load_res = engine.execute(QueryParser::parse("LOAD emp FROM 'test_data.csv'"));
    assert(std::holds_alternative<std::string>(load_res));

    // Test SELECT
    auto select_res = engine.execute(QueryParser::parse("SELECT * FROM emp WHERE id = '1'"));
    assert(std::holds_alternative<Table>(select_res));
    auto& results = std::get<Table>(select_res);
    assert(results.size() == 1);
    assert(results[0].at("name") == "Alice");

    std::cout << "test_engine_execution passed!" << std::endl;
}

int main() {
    try {
        test_utils();
        test_parser();
        test_engine_execution();
        std::cout << "\nALL TESTS PASSED SUCCESSFULLY!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}