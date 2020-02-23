#pragma once

#include <string>
#include <memory>

using namespace std;

class OCRBuilder
{
protected:
    OCRBuilder()
    {
    }
public:
    virtual ~OCRBuilder()
    {
    }

    virtual OCRBuilder* setPath(const string& path) = 0;
    virtual OCRBuilder* setGrayRange(int min, int max) = 0;
    virtual string ocr() = 0;
};

unique_ptr<OCRBuilder> createOCRBuilder();

void rotate(const string& path, float rot, const vector<int>& clip);