#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED
#endif // STACK_H_INCLUDED

const size_t CAP = 100;
const size_t STACK_REALLOC_LIMIT = 10000;

const int STACK_OVERFLOW_ERROR = 11; //function, not stack errors
const int STACK_UNDERFLOW_ERROR = 12;
const int STACK_REALLOC_ERROR = 2;
const int STACK_CLEAN_ERROR = 3;
const int OPEN_FILE_ERROR = -2;

const int STACK_SEG_FAULT_ERROR = 1; //Stack errors
const int STACK_CAN_struct_ERROR = 7;
const int STACK_CAN_Data_ERROR = 8;
const int STACK_UNID_ERROR = -666;
const int STACK_HASH_ERROR = -13;

const int READY = 0;
const int NOT_INIT = -1;

typedef value T;

class Stack
{
    private:
        T *Data = NULL;
        size_t Size = 0;
        size_t Capacity = 0;
        int realloc ();
    public:
        int Status = -1;
        void init (); //deal with memory
        void destroy ();

        int push (T value); //no-deal with memory
        int pop (T *value);
        int top (T *value);
        int filewrite (const char* File);
        int hard_dump_struct (const char* File);
        int hard_dump_Data (const char* File);
};

#include "Stack.cpp"
