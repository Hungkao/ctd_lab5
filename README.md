1. Cách build chương trình


cd incompleted
make


Sau khi build thành công, file thực thi kplc sẽ được tạo ra.

2. Cách chạy chương trình


Chạy một test đơn lẻ


./kplc ../tests/example6.kpl

3. Chạy toàn bộ test


cd ..
chmod +x test.sh
./test.sh


Kết quả symbol table của từng chương trình sẽ được in trực tiếp ra màn hình


Kết quả khi chạy full 


hungkao@DESKTOP-FCV8LT8:~/ctd/ctd_lab5/incompleted$ cd ..
chmod +x test.sh
./test.sh
===== BUILD PROJECT =====
make: Nothing to be done for 'all'.

===== RUN ALL TESTS =====

==============================
TEST FILE: tests/example1.kpl
------------------------------

Program EXAMPLE1

==============================


==============================

TEST FILE: tests/example2.kpl

------------------------------

Program EXAMPLE2
    Var N : Int
    Function F : Int
        Param N : Int

==============================

==============================

TEST FILE: tests/example3.kpl

------------------------------

Program EXAMPLE3
    Var I : Int
    Var N : Int
    Var P : Int
    Var Q : Int
    Var C : Char
    Procedure HANOI
        Param N : Int
        Param S : Int
        Param Z : Int

==============================

==============================

TEST FILE: tests/example4.kpl

------------------------------

Program EXAMPLE4
    Const MAX = 10
    Type T = Int
    Var A : Arr(10,Int)
    Var N : Int
    Var CH : Char
    Procedure INPUT
        Var I : Int
        Var TMP : Int

    Procedure OUTPUT
        Var I : Int

    Function SUM : Int
        Var I : Int
        Var S : Int

==============================

==============================

TEST FILE: tests/example5.kpl

------------------------------


Program EXAMPLE5
    Const C = 1
    Type T = Char
    Function F : Char
        Param I : Int
        Const B = 0
        Type A = Arr(5,Int)

==============================

==============================

TEST FILE: tests/example6.kpl

------------------------------

Program EXAMPLE6
    Const C1 = 10
    Const C2 = 'a'
    Type T1 = Arr(10,Int)
    Var V1 : Int
    Var V2 : Arr(10,Int)
    Function F : Int
        Param P1 : Int
        Param VAR P2 : Char

    Procedure P
        Param V1 : Int
        Const C1 = 'a'
        Const C3 = 10
        Type T1 = Int
        Type T2 = Arr(10,Int)
        Var V2 : Int
        Var V3 : Char

==============================



===== DONE =====
