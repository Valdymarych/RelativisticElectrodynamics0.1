#pragma once
#include <iostream>

class Logger{
    public:
        inline static int depth=0;
        inline static int off_depth=20;
        template<typename... Args>
        static void begin(Args... args){
            if (Logger::off_depth>Logger::depth){
                for (int i=0;i<Logger::depth;i++){
                    std::cout<<" - ";
                }
                (std::cout << ... << args)<<std::endl;
            }
            Logger::depth++;
        }
        static void off(){
            Logger::off_depth=Logger::depth;
        }
        template<typename... Args>
        static void log(Args... args){
            if (Logger::off_depth>Logger::depth){
                Logger::flog(args...);
            }
        }
        template<typename... Args>
        static void flog(Args... args){
            for (int i=0;i<Logger::depth;i++){
                std::cout<<" - ";
            }
            (std::cout << ... << args)<<std::endl;
        }
        template<typename... Args>
        static void end(Args... args){
            if (Logger::depth>0){
                Logger::depth--;
            }
            if (Logger::off_depth>Logger::depth){
                Logger::off_depth=20;
                for (int i=0;i<Logger::depth;i++){
                    std::cout<<" - ";
                }
                (std::cout << ... << args)<<std::endl;
            }
            if (Logger::off_depth==Logger::depth){
                Logger::off_depth=20;
            }
        }
};
