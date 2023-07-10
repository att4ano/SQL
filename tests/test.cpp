#include <gtest/gtest.h>
#include "lib/db.h"

TEST(DataBase, CreateTableTest) {
    DataBase DataBase("Test");
    std::string sql_request1 = "CREATE TABLE orders (order_id INT PRIMARY KEY NOT NULL, supplier_id INT NOT NULL, order_date VARCHAR(20));";
    DataBase.CreateTable(sql_request1);

    ASSERT_EQ(DataBase.GetTables().size(), 1);
    ASSERT_TRUE(DataBase.GetTables().find("orders") != DataBase.GetTables().end());

    std::string sql_request2 = "CREATE TABLE suppliers (supplier_id INT PRIMARY KEY NOT NULL, supplier_name VARCHAR(20));";
    DataBase.CreateTable(sql_request2);

    ASSERT_EQ(DataBase.GetTables().size(), 2);
    ASSERT_TRUE(DataBase.GetTables().find("suppliers") != DataBase.GetTables().end());
}

TEST(DataBase, DropTableTest) {
    DataBase DataBase("Test");
    std::string sql_request1 = "CREATE TABLE orders (order_id INT PRIMARY KEY NOT NULL, supplier_id INT NOT NULL, order_date VARCHAR(20));";
    DataBase.CreateTable(sql_request1);

    std::string sql_request2 = "DROP TABLE orders;";
    DataBase.DropTable(sql_request2);
    ASSERT_EQ(DataBase.GetTables().size(), 0);
}

TEST(DataBase, InsertInTableTest) {
    DataBase DataBase("Test");
    std::string sql_request1 = "CREATE TABLE orders (order_id INT PRIMARY KEY NOT NULL, supplier_id INT NOT NULL, order_date VARCHAR(20));";
    DataBase.CreateTable(sql_request1);
    sql_request1 = "INSERT INTO orders VALUES (125, 0, \"05.05.2015\");";
    DataBase.Insert(sql_request1);
    sql_request1 = "INSERT INTO orders VALUES (126, 1, \"08.02.2016\");";
    DataBase.Insert(sql_request1);
    ASSERT_EQ(DataBase.GetTables()["orders"].GetElement().size(), 2);
    for (size_t i = 0; i < 2; ++i) {
        if (i == 0) {
            ASSERT_EQ(DataBase.GetTables()["orders"].GetElement()[i]["order_id"].GetValue<int>(), 125);
            ASSERT_EQ(DataBase.GetTables()["orders"].GetElement()[i]["supplier_id"].GetValue<int>(), 0);
            ASSERT_EQ(DataBase.GetTables()["orders"].GetElement()[i]["order_date"].GetValue<std::string>(), "05.05.2015");
        } else {
            ASSERT_EQ(DataBase.GetTables()["orders"].GetElement()[i]["order_id"].GetValue<int>(), 126);
            ASSERT_EQ(DataBase.GetTables()["orders"].GetElement()[i]["supplier_id"].GetValue<int>(), 1);
            ASSERT_EQ(DataBase.GetTables()["orders"].GetElement()[i]["order_date"].GetValue<std::string>(), "08.02.2016");
        }
    }
}

TEST(DataBase, UpdateTest) {
    DataBase DataBase("Test");

    std::string sql_request1 = "CREATE TABLE orders (order_id INT PRIMARY KEY NOT NULL, supplier_id INT NOT NULL, order_date VARCHAR(20));";
    DataBase.CreateTable(sql_request1);
    sql_request1 = "INSERT INTO orders VALUES (125, 0, \"05.05.2015\");";
    DataBase.Insert(sql_request1);
    sql_request1 = "INSERT INTO orders VALUES (126, 1, \"08.02.2016\");";
    DataBase.Insert(sql_request1);
    sql_request1 = "UPDATE orders SET order_id = 228 WHERE supplier_id = 0;";
    DataBase.UpdateRequest(sql_request1);
    ASSERT_EQ(DataBase.GetTables()["orders"].GetElement()[0]["order_id"].GetValue<int>(), 228);
}

TEST(DataBase, DeleteTest) {
    DataBase DataBase("Test");
    std::string sql_request1 = "CREATE TABLE orders (order_id INT PRIMARY KEY NOT NULL, supplier_id INT NOT NULL, order_date VARCHAR(20));";
    DataBase.CreateTable(sql_request1);
    sql_request1 = "INSERT INTO orders VALUES (125, 0, \"05.05.2015\");";
    DataBase.Insert(sql_request1);
    sql_request1 = "INSERT INTO orders VALUES (126, 1, \"08.02.2016\");";
    DataBase.Insert(sql_request1);
    sql_request1 = "DELETE FROM orders WHERE order_id = 125;";
    DataBase.DeleteRequest(sql_request1);
    ASSERT_EQ(DataBase.GetTables()["orders"].GetElement().size(), 1);
    ASSERT_EQ(DataBase.GetTables()["orders"].GetElement()[0]["order_id"].GetValue<int>(), 126);
}

