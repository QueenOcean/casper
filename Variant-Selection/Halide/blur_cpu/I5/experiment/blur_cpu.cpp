// Halide tutorial lesson 1: Getting started with Funcs, Vars, and Exprs

// This lesson demonstrates basic usage of Halide as a JIT compiler for imaging.


// g++ blur_cpu.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o blur_cpu -std=c++11

// LD_LIBRARY_PATH=../bin ./blur_cpu



// The only Halide header file you need is Halide.h. It includes all of Halide.
#include "Halide.h"
#include "clock.h"


using namespace Halide;

// We'll also include stdio for printf.
#include <stdio.h>
#include <cmath>
#include <random>
#include <iostream>
#include <fstream>






int main(int argc, char **argv) {

    std::string FILENAME = "halide_blur_cpu_Xeon_2^10.csv"; // here
    std::ofstream ofile;
    ofile.open(FILENAME);

    int NUM = 20;

    for (int k = 0; k <= 0; k++){ // here

        int p_input = 10 + k;

        for (int i = 0; i < NUM; i++) {

            if (i % 10 == 0) std::cout << i << std::endl;
        
            Func blur_x, blur_y;
            Var x_, y_, xi, yi;

            Func input;
            input(x_,y_) = rand() % 1024 + 1;
            
            int power = 10;
            int p1 = rand() % power + 1; 
            int p2 = rand() % power + 1; 
            int p3 = rand() % p2 + 1; // p2 > p3
            int p4 = rand() % p3 + 1; // p3 > p4

            /*
            int v1 = pow(2,p1);
            int v2 = pow(2,p2);
            int v3 = pow(2,p3);
            int v4 = pow(2,p4);
            */

            int v1 = 8;
            int v2 = 256;
            int v3 = 128;
            int v4 = 8;
            

            std::cout << v1 << "," << v2 << "," << v3 << "," << v4 << std::endl;



            // The algorithm - no storage or order
            blur_x(x_, y_) = (input(x_-1, y_) + input(x_, y_) + input(x_+1, y_))/3;
            blur_y(x_, y_) = (blur_x(x_, y_-1) + blur_x(x_, y_) + blur_x(x_, y_+1))/3;

            // The schedule - defines order, locality; implies storage

            Var x_i("x_i");
            Var x_i_vi("x_i_vi");
            Var x_i_vo("x_i_vo");
            Var x_o("x_o");
            Var x_vi("x_vi");
            Var x_vo("x_vo");
            Var y_i("y_i");
            Var y_o("y_o");


            {
                Var x = blur_x.args()[0];
                blur_x
                    .compute_at(blur_y, x_o)
                    .split(x, x_vo, x_vi, v1)
                    .vectorize(x_vi);
            }
            {

            
                Var x = blur_y.args()[0];
                Var y = blur_y.args()[1];
                blur_y
                    .compute_root()
                    .split(x, x_o, x_i, v2)
                    .split(y, y_o, y_i, v3)
                    .reorder(x_i, y_i, x_o, y_o)
                    .split(x_i, x_i_vo, x_i_vi, v4)
                    .vectorize(x_i_vi)
                    .parallel(y_o)
                    .parallel(x_o);
            }


            double time = 0;
            int rounds = 1; // here

            for (int j = 0; j < rounds; j++){
                
                double t1 = current_time();
                // blur_y.realize(pow(2,p_input),pow(2,p_input)); // here
                blur_y.realize(pow(2,13),pow(2,13)); // here
                double t2 = current_time();
                
                time += (t2 - t1)/1000;
            }

            double avgtime = time/rounds; 

            std::cout << avgtime << std::endl;

            ofile << avgtime << "," << pow(2,10) << "," << v1 << "," << v2 << "," << v3 << "," << v4 << "," << '\n';
        }
    }


    ofile.close();

    // Everything worked! We defined a Func, then called 'realize' on
    // it to generate and run machine code that produced an Buffer.
    printf("Success!\n");

    return 0;
}