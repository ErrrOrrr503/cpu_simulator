void Stack::init ()
{
    this->Data = new T[CAP];
    if (this->Data == NULL) {
        std::cout << "ERROR INITING at Stack! Memory is not allocared!\n";
        return;
    }
    this->Size = 0;
    this->Capacity = CAP;
    this->Status = READY;
}

void Stack::destroy ()
{
    delete[] this->Data;
    this->Size = 0;
    this->Capacity = 0;
    this->Status = NOT_INIT;
    return;
}

int Stack::realloc ()
{
    size_t New_Size = 0;
    if (this->Capacity >= STACK_REALLOC_LIMIT)
        New_Size = STACK_REALLOC_LIMIT + this->Capacity;
    else
        New_Size = 2 * this->Capacity;

    T *New_Data = new T[New_Size];
    if (New_Data == NULL) {
        std::cout << "ERROR REALLOCING at Stack!\n";
        return STACK_REALLOC_ERROR;
    }
    std::memcpy (New_Data, this->Data, this->Capacity * sizeof (T));
    delete[] this->Data;
    this->Data = New_Data;
    this->Capacity = New_Size;
    return 0;
}

int Stack::push (T value)
{
    if (this->Size > this->Capacity - 1)
        if (this->realloc () == STACK_REALLOC_ERROR) {
            std::cout << "ERROR OVERFLOW at Stack!\n";
            this->Status = STACK_OVERFLOW_ERROR;
            return STACK_OVERFLOW_ERROR;
        }
    this->Data[this->Size] = value;
    this->Size++;
    return 0;
}

int Stack::pop (T *value)
{
    this->Size--;
    if (this->Size > this->Capacity - 1) {
        //std::cout << "ERROR UNDERFLOW at Stack by Stack_pop()!\n";
        this->Size++;
        this->Status = STACK_UNDERFLOW_ERROR;
        return STACK_UNDERFLOW_ERROR;
    }
    *value = this->Data[this->Size];
    return 0;
}

int Stack::top (T *value)
{
    this->Size--;
    if (this->Size > this->Capacity - 1) {
        std::cout << "ERROR UNDERFLOW at Stack by Stack_top()!\n";
        this->Size++;
        this->Status = STACK_UNDERFLOW_ERROR;
        return STACK_UNDERFLOW_ERROR;
    }
    *value = this->Data[this->Size];
    this->Size++;
    return 0;
}

int Stack::filewrite (const char* File)
{
    std::ofstream Stack_out (File);
    if (!Stack_out) {
        std::cout << "ERROR opening file in Stack_fwrite\n";
        return OPEN_FILE_ERROR;
    }
    Stack_out << "Stack.Data* = " << (size_t) this->Data << std::endl;
    Stack_out << "Stack.Size =     " << this->Size << std::endl << "Stack.Capacity = " << this->Capacity << std::endl;
    for (size_t i = 0; i < this->Capacity; i++) {
        Stack_out << "[" << i << "/" << this->Capacity - 1 << "] = " << this->Data[i].int_n << std::endl;
    }
    std::cout << "Stack written successfilly) Is it that bad that you used me?\n";
    Stack_out.close();
    return 0;
}

int Stack::hard_dump_struct (const char* File)
{
    FILE* dump = NULL;
    dump = fopen (File, "wb");
    if (dump == NULL) {
        std::cout << "ERROR even while hard_dumping, it seems the processor exploaded...\n";
        return OPEN_FILE_ERROR;
    }
    char *dump_buf = new char[sizeof (*this)];
    std::memcpy (dump_buf, this, sizeof (*this));
    fwrite (dump_buf, sizeof (char), sizeof (*this), dump);
    fclose (dump);
    return 0;
}

int Stack::hard_dump_Data (const char* File)
{
    FILE* dump = NULL;
    dump = fopen (File, "wb");
    if (dump == NULL) {
        std::cout << "ERROR even while hard_dumping, be careful, your RAM must be burning right now...\n";
        return OPEN_FILE_ERROR;
    }
    T *dump_buf = new T[this->Capacity];
    std::memcpy (dump_buf, this->Data, this->Capacity * sizeof (T));
    fwrite (dump_buf, sizeof (char), this->Capacity * sizeof (T), dump);
    fclose (dump);
    return 0;
}
