#  Project B - CSV Mini Database and Query Engine

## Description

Project B is a lightweight, CSV-based mini database and query engine designed to provide basic database functionality using CSV (Comma-Separated Values) files as the primary data storage format. This project demonstrates fundamental concepts in database management systems, including data loading, querying, and manipulation, without relying on traditional relational databases.

The system allows users to treat CSV files as database tables, enabling simple yet powerful operations such as data retrieval, insertion, updates, and deletions through a custom query language inspired by SQL.

## Key Features

- **CSV Data Loading**: Automatically load and parse CSV files into in-memory table structures with type inference for columns.
- **Query Language**: Support for basic SQL-like queries including:
  - SELECT statements with WHERE clauses
  - INSERT operations to add new records
  - UPDATE operations to modify existing data
  - DELETE operations to remove records
- **Data Types**: Handle common data types including integers, floats, strings, booleans, and dates.
- **Indexing**: Optional indexing mechanisms for improved query performance on larger datasets.
- **Command-Line Interface**: A user-friendly CLI for executing queries and managing the database.
- **Export Functionality**: Ability to export query results back to CSV format.

## Architecture

The project is structured into several key components:

- **Parser**: Handles query parsing and syntax validation.
- **Engine**: Executes queries on the loaded CSV data.
- **Storage Manager**: Manages in-memory representation of CSV tables.
- **CLI**: Provides the user interface for interaction.

## Usage Example

```bash
# Load a CSV file as a table
LOAD employees FROM 'employees.csv'

# Query the data
SELECT name, salary FROM employees WHERE department = 'Engineering'

# Insert new data
INSERT INTO employees VALUES ('John Doe', 'Engineering', 75000)

# Update existing records
UPDATE employees SET salary = 80000 WHERE name = 'Jane Smith'
```

## Requirements

- Python 3.8+
- Standard library modules (csv, os, etc.)
- Optional: pandas for advanced data manipulation

## Getting Started

1. Clone the repository
2. Install dependencies (if any)
3. Run the main script: `python main.py`
4. Use the CLI to load CSV files and execute queries

## Project Goals

This project serves as an educational tool to understand:
- Database query processing
- Data storage and retrieval mechanisms
- Implementation of a simple query language
- Memory management for data structures