# Test Header 

This README.md was create against my will. If you find yourself did not understand anything after you read this, just bare with it.

Writing unit test case was just like wiping your butt right after shit. I hate it but I have to do it . 

Writing unit test itself was disgusting enough though. But I can`t find a good toilet paper ! 

No unit test framework satisfied me until now. I just want a test framework that was simple enough to just put them in a single header file.

Since nobody was doing this, I`m gonna do it myself 

**FYI**
**This framework supports Linux only.Other system? Later....**

## Synopsis

6 step , and you`re ready to go

#### 1. Clone this repo
```
git clone https://github.com/Rosanta/testHeader.git
```
Or, you can just get the header file.
```
wget https://github.com/Rosanta/testHeader/blob/master/test.h
```

#### 2. Create a new file and include the header
```
#include "test.h"
```

#### 3. Name this test package
I`m gonna name it "PACKAGE1" for example

```
#include "test.h"

TEST_PACKAGE(PACKAGE1)
```


#### 4. Add test cases

```
#include "test.h"

TEST_PACKAGE(PACKAGE1)

TEST(CASE1) {
    ASSERT(1);
    ASSERT(true);
    TEST_LOG("LOG ME");
}
```
#### 5. Tell me when you`re ready

Once you add all cases, always add "START_TEST()" in the end of the file. So the framework could know you are ready

```
#include "test.h"

TEST_PACKAGE(PACKAGE1)

TEST(CASE1) {
    ASSERT(1);
    ASSERT(true);
    TEST_LOG("LOG ME");
}

START_TEST();
```

#### 6.Compile and run

Compile the code and run. No additional libs would be required.


**you can find a example code in the "example" directory**


## Interfaces

All interfaces was defined in the beginning of the "test.h" file.



* TEST_PACKAGE(NAME)

 Define a new test package.  One file can only invoke it once ! It means one file can only have one test package.
 
* TEST(NAME) {.../* code */..}
    
    Define a test case
    
* START_TEST()

    Notify the framework that all test cases was finished. A test file should always end with this function. If you want to do the test
    
* TEST_LOG(fmt, ...) 

    Call this function if you want to print something in a test case. It is supports "printf"-like parameters
 
 
* TEST_ASSERT(exp)
    
    Test if the vale of the "exp" argument was TRUE. If it fails, the test would be terminated .
    
    
    



## License 
This project is licensed under the **WTFPL** . You can find a copy of it in LICENSE.txt



