#include <iostream>
#include <windows.h>
#include "lib/db.h"

int main() {
    std::string input = "CREATE TABLE suppliers (supplier_id INT PRIMARY KEY NOT NULL, supplier_name VARCHAR(20));";
    DataBase d("123");
    d.CreateTable(input);
    input = "CREATE TABLE orders (order_id INT PRIMARY KEY NOT NULL, supplier_id INT NOT NULL, order_date VARCHAR(20));";
    d.CreateTable(input);
    input = "INSERT INTO suppliers VALUES (0, \"IBM\");";
    d.Insert(input);
    input = "INSERT INTO suppliers VALUES (1, \"HP\");";
    d.Insert(input);
    input = "INSERT INTO suppliers VALUES (2, \"Microsoft\");";
    d.Insert(input);
    input = "INSERT INTO suppliers VALUES (3, \"NVidia\");";
    d.Insert(input);
    input = "INSERT INTO orders VALUES (125, 0, \"05.05.2015\");";
    d.Insert(input);
    input = "INSERT INTO orders VALUES (126, 1, \"08.02.2016\");";
    d.Insert(input);
    input = "INSERT INTO orders VALUES (127, 4, \"06.01.2017\");";
    d.Insert(input);
//    std::string sqlQuery = "SELECT suppliers.supplier_id, suppliers.supplier_name, orders.order_date FROM suppliers LEFT JOIN orders ON suppliers.supplier_id = orders.supplier_id;";
    std::string sql_request = "SELECT * FROM orders WHERE order_id = 126 OR order_id = 127;";
    d.SelectRequest(sql_request);
    return 0;
}
