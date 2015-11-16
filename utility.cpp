//
// Created by Di Tian on 11/13/15.
//
#include <mysql.h>
#include <iostream>

#include "utility.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
using namespace std;

MYSQL* initialize() {
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr) {
        cout << "Insufficient memory!" << endl;
        exit(0);
    }
    return conn;
}

void connect(MYSQL* conn) {
    if (conn == nullptr) {
        cout << "Null pointer." << endl;
        exit(0);
    }
    if (mysql_real_connect(conn, "localhost", "root", "12345", "project3-nudb", 0, 0, 0) == nullptr) {
        cout << "Connection failed!" << endl;  // unable to connect
    }
    /*
    else {
        cout << "Connection succeeded!" << endl;
    }
    */
}

void close(MYSQL* conn) {
    if (conn == nullptr) {
        cout << "Null pointer." << endl;
        exit(0);
    }
    mysql_close(conn);
}

void error(MYSQL* conn) {
    cout << "Could not execute statement(s).";
    mysql_close(conn);
    exit(0);
}

MYSQL_RES* send_query(string query) {
    MYSQL* conn = initialize();
    connect(conn);
    if (mysql_query(conn, query.c_str())) {
        error(conn);
    }

    MYSQL_RES* res_set = mysql_store_result(conn);
    close(conn);
    return res_set;
}

void course_detail(string id, string course) {
    string query = "select uoscode, unitofstudy.uosname, semester, year, enrollment, maxenrollment, faculty.name, grade"
                   " from transcript join uosoffering using (Uoscode, Semester, Year)"
                   " join unitofstudy using (Uoscode)"
                   " join faculty on (instructorid = id)"
                   " where studid = " + id + " and uoscode = \"" + course +
                   "\" order by year, semester;";

    MYSQL_RES *res_set = send_query(query);

    int num_rows = (int) mysql_num_rows(res_set);
    for(int i = 0; i < num_rows; i++) {
        MYSQL_ROW row = mysql_fetch_row(res_set);
        cout << endl;
        cout << "Course: " << row[0] << "-" << row[1] << endl;
        cout << "Quarter: " << row[3] << " " << row[2] << endl;
        cout << "# Enrolled/Max: " << row[4] << "/" << row[5] << endl;
        cout << "Lecturer: " << row[6] << endl;
        cout << "Grade: ";
        if (row[7] == nullptr) {
            cout << "<<< Not Yet Graded >>>" << endl;
        } else {
            cout << row[7] << endl;
        }
        cout << endl;
    }

    mysql_free_result(res_set);

    cout << "Press any key to go back" << endl;
    string any;
    cin >> any;
}

void transcript(string id) {
    while(true) {
        cout << " ----------------------------" << endl;
        cout << "| Your unofficial transcript |" << endl;
        cout << " ----------------------------" << endl;

        string query = "select * from transcript where StudId = " + id + " order by year, semester;";
        MYSQL_RES *res_set = send_query(query);

        int num_rows = (int) mysql_num_rows(res_set);
        for (int i = 0; i < num_rows; i++) {
            MYSQL_ROW row = mysql_fetch_row(res_set);
            cout << row[1] << "  " << row[2] << "  " << row[3] << "  ";
            if (row[4] == nullptr) {
                cout << "<<< Not Yet Graded >>>" << endl;
            } else {
                cout << row[4] << endl;
            }
        }

        mysql_free_result(res_set);

        cout << endl << "Type any course number above to view course details;" << endl;
        cout << "Or type \"0\" to go back." << endl;

        string course;
        cin >> course;  //TODO: 1. type check; 2. non-exist course number
        if (course == "0") {
            return;
        } else {
            course_detail(id, course);
        }
    }
}

void enroll(LoginInfo* info) {
    string id = info->GetId();
    int cur_q_year = info->GetCurrentQuarterPtr()->GetQuarter_SchoolYear();
    string cur_q_name = info->GetCurrentQuarterPtr()->GetQuarter_Name();
    int next_q_year = info->GetNextQuarterPtr()->GetQuarter_SchoolYear();
    string next_q_name = info->GetNextQuarterPtr()->GetQuarter_Name();
    string stmt_str = "CALL candidate_course(" + id + ", " + to_string(cur_q_year) + ", \"" + cur_q_name + "\", " +
                  to_string(next_q_year) + ", \"" + next_q_name + "\");";

    MYSQL_RES* res_set = send_query(stmt_str);

    int num_rows = (int) mysql_num_rows(res_set);
    if (num_rows == 0) {
        cout << "Yeah! You are not eligible to enroll in any course." << endl;
    } else {
        cout << " ----------------------------------------------------------" << endl;
        cout << "| You might be eligible to enroll in the following courses |" << endl;
        cout << " ----------------------------------------------------------" << endl;
        for (int i=0; i<num_rows; ++i) {
            MYSQL_ROW row = mysql_fetch_row(res_set);
            cout << row[0] <<  "  " << row[2] << "  " << row[3] << "  " << row[1] << endl;
        }
        cout << endl;
        //cout << "--------------------------------------------" << endl;
    }
    mysql_free_result(res_set);

    string new_course;
    int new_course_year;
    string new_course_quarter;
    cout << "Please input the course code: ";
    cin >> new_course;
    cout << "Please input the year of the new course: ";
    cin >> new_course_year;
    cout << "Please input the quarter of the new course: ";
    cin >> new_course_quarter;

    string new_course_begins_on = to_string(new_course_year) + "-";
    if (new_course_quarter=="Q1" || new_course_quarter=="q1") {
        new_course_begins_on += "09-15";
    } else if (new_course_quarter=="Q2" || new_course_quarter=="q2") {
        new_course_begins_on += "01-05";
    } else if (new_course_quarter=="Q3" || new_course_quarter=="q3") {
        new_course_begins_on += "03-30";
    } else if (new_course_quarter=="Q4" || new_course_quarter=="q4") {
        new_course_begins_on += "06-22";
    } else {
        new_course_begins_on += "12-31";
    }
    string stmt_str_1 = "CALL enroll(" + id + ", \"" + new_course + "\", " + to_string(new_course_year) + ", \"" +
                        new_course_quarter + "\", \"" + new_course_begins_on + "\", " + "@status);";
    // cout << stmt_str_1 << endl;
    //std::auto_ptr<sql::Statement>

}

void student_menu(LoginInfo* info) {
    int month = info->GetMonth();
    int day = info->GetDay();
    int year = info->GetYear();
    string weekday_name = info->GetNameOfWeekDay();

    string id = info->GetId();
    int school_year = info->GetCurrentQuarterPtr()->GetQuarter_SchoolYear();
    string quarter = info->GetCurrentQuarterPtr()->GetQuarter_Name();

    while (true) {
        cout << endl << "You are logged in as " <<  id << "." << endl;
        cout << "Today is " << month << "-" << day << "-" << year << ", " << weekday_name << "." << endl << endl;
        string stmt_str = "select unitofstudy.UoSCode, unitofstudy.UoSName, transcript.Grade"
                " from student"
                " join transcript on student.Id = transcript.StudId"
                " join uosoffering on transcript.UoSCode = uosoffering.UoSCode and"
                " transcript.Semester = uosoffering.Semester and"
                " transcript.Year = uosoffering.Year"
                " join unitofstudy on transcript.UoSCode = unitofstudy.UoSCode";
        //" where student.Id = \"%s\" and uosoffering.Year = %d and uosoffering.Semester = \"%s\";";
        stmt_str += " where student.Id = " + id +
                    " and uosoffering.Year = " + to_string(school_year) +
                    " and uosoffering.Semester = \"" + quarter + "\";";

        MYSQL_RES* res_set = send_query(stmt_str);
        int num_rows = (int) mysql_num_rows(res_set);
        if (num_rows == 0) {
            cout << "Yeah! You are not enrolled in any course!" << endl;
        } else {
            cout << " -------------------------------------------" << endl;
            cout << "| You are enrolled in the following courses |" << endl;
            cout << " -------------------------------------------" << endl;
            for (int i=0; i<num_rows; ++i) {
                MYSQL_ROW row = mysql_fetch_row(res_set);
                cout << row[0] <<  "    " << row[1];
                if (row[2] == nullptr) {
                    cout << "    <<< Not Yet Graded >>>";
                }
                cout << endl;
            }
            cout << endl;
            //cout << "--------------------------------------------" << endl;
        }
        mysql_free_result(res_set);

        cout << " -------------------------------------------" << endl;
        cout << "| You can proceed to the following options  |" << endl;
        cout << " -------------------------------------------" << endl;
        cout << "1. Transcript" << endl << "2. Enroll" << endl << "3. Withdrow" << endl
        << "4. Personal Details" << endl << "5. Logout" << endl;
        cout << endl;
        //cout << "--------------------------------------------" << endl;
        cout << "Please select: ";
        int option = 0;
        cin >> option;  // TODO: type check
        if (option == 1) {
            transcript(id);
        } else if (option == 2) {
            enroll(info);
        } else if (option == 3) {
            // TODO: withdraw
        } else if (option == 4) {
            // TODO: personal details
        } else if (option == 5) {
            cout << "Bye!" << endl << endl;
            delete info;  // deallocate the LoginInfo object assigned to current user
            login();
        } else {
            cout << "Invalid option. Please reselect." << endl;
        }
    }
}

void login() {

    while (true) {
        string student_id;
        string password;
        cout << "Username: ";  // TODO: 1). int type check; 2). int length check.
        cin >> student_id;
        cout << "Password: ";
        cin >> password;

        LoginInfo* info = new LoginInfo(student_id);

        string query = "select * from student where id = " + student_id;
        MYSQL_RES *res_set = send_query(query);
        MYSQL_ROW row;
        int num_rows = (int) mysql_num_rows(res_set);

        if (num_rows == 0) {
            cout << "Student doesn't exist!" << endl;
        } else {
            row = mysql_fetch_row(res_set);
            if (row[2] == password) {
                student_menu(info);
                break;
            } else {
                cout << "Wrong password:(" << endl;
            }
        }
        mysql_free_result(res_set);

        delete info;
    }
}

#pragma clang diagnostic pop