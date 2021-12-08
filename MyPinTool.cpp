/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs 
 *  and could serve as the starting point for developing your first PIN tool
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <map>
#include <list>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

/* ================================================================== */
// Global variables
/* ================================================================== */

UINT64 insCount    = 0; //number of dynamically executed instructions
UINT64 bblCount    = 0; //number of dynamically executed basic blocks
UINT64 rountineCount = 0; //total number of routine invocations, including main thread

std::ostream* out = &cout;
std::ostream* imageLoadLog = &cout;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "", "Specify file name for call tracer output.");
KNOB< string > KnobImageLoadLog(KNOB_MODE_WRITEONCE, "pintool", "s", "", "Specify file name for symbol table tracer output.");
KNOB< BOOL > KnobLogSymbolTable(KNOB_MODE_WRITEONCE, "pintool", "sy", "0", "Specify if you want image load tracer to write out symbol table as well.");

KNOB< BOOL > KnobCount(KNOB_MODE_WRITEONCE, "pintool", "count", "1",
                       "count instructions, basic blocks and threads in the application");


std::map<THREADID, std::list<string>> threadFunctionCalls;
std::map<ADDRINT, std::string> funcnames;


/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    cerr << "This tool prints out the number of dynamically executed " << endl
         << "instructions, basic blocks and threads in the application." << endl
         << endl;

    cerr << KNOB_BASE::StringKnobSummary() << endl;

    return -1;
}


/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */


VOID ThreadStart(THREADID threadIndex, CONTEXT* ctxt, INT32 flags, VOID* v) {
    std::list<string> functionTrace;
    threadFunctionCalls[PIN_ThreadId()] = functionTrace;
}


VOID ThreadFini(THREADID threadid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    *out << "=====================================" << endl;
    *out << "Thread Function Trace: " << threadid << endl;
    for (string functionName: threadFunctionCalls[threadid])
        *out << "\t" << functionName << endl;
    *out << "=====================================" << endl;
}


VOID Fini(INT32 code, VOID* v)
{
    *out << "===============================================" << endl;
    *out << "finished tracing "<< rountineCount << " routines" << endl;
    *out << "===============================================" << endl;
}

VOID AddNewLayerTrace(VOID* name) {
    rountineCount++;
    threadFunctionCalls[PIN_ThreadId()].push_back(PIN_UndecorateSymbolName((char*)name, UNDECORATION_NAME_ONLY));     
}

VOID InjectFunctionNameTracer(RTN rtn, VOID* v) {
    RTN_Open(rtn);
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)AddNewLayerTrace, IARG_PTR, (VOID*) &RTN_Name(rtn), IARG_END);
    RTN_Close(rtn);

}

// TODO: Add function instrumentation for loaded libraries as well!
// Injected logic on each image load into address space (each time executable or DLL is loaded.)
VOID ImageLoadTracer(IMG img, VOID* v) {
    // When an image is loaded into the address space, document all of its function symbols to the log.
    // Useful for debugging.
    if (!IMG_Valid(img)) return;

    //*imageLoadLog << "Loaded image: " << IMG_Name(img) << endl;

    // If we opted to record the symbol table as well, we write it out to disk as well.


    if (KnobLogSymbolTable) {
        // Iterate over sections
        for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
            // Iterate over routines
            for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
                ADDRINT routine_addr = RTN_Address(rtn);
                std::string routine_name = RTN_Name(rtn);
                funcnames[routine_addr] = routine_name;
                *imageLoadLog << "\t" << routine_addr << ": " << routine_name << endl;
            }
        }
    }
}

// TODO: Add function instrumentation for loaded libraries as well!
// Injected logic on each image load into address space (each time executable or DLL is loaded.)
VOID ImageUnloadTracer(IMG img, VOID* v) {
    // When an image is unloaded from the address space, document it in the logs.
    // Useful for debugging.
    if (!IMG_Valid(img)) return;

    *imageLoadLog << "Unloaded image: " << IMG_Name(img) << endl;
}


/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments, 
 *                              including pin -t <toolname> -- ...
 */
int main(int argc, char* argv[])
{
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    // PIN_InitSymbols();
    PIN_InitSymbolsAlt(DEBUG_OR_EXPORT_SYMBOLS);

    

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    string fileName = KnobOutputFile.Value();

    if (!fileName.empty())
    {
        out = new std::ofstream(fileName.c_str());
    }

    string imageLoadLogFile = KnobImageLoadLog.Value();

    if (!imageLoadLogFile.empty())
    {
        imageLoadLog = new std::ofstream(imageLoadLogFile.c_str());
    }

    if (KnobCount)
    {
        // Register function to be called to instrument traces
        // TRACE_AddInstrumentFunction(Trace, 0);

        // Register function to be called for every thread before it starts running
        //PIN_AddThreadStartFunction(ThreadStart, 0);

        // Register function to be called on every function call
        RTN_AddInstrumentFunction(InjectFunctionNameTracer, 0);

        IMG_AddInstrumentFunction(ImageLoadTracer, 0);

        IMG_AddUnloadFunction(ImageUnloadTracer, 0);

        PIN_AddThreadStartFunction(ThreadStart, 0);

        PIN_AddThreadFiniFunction(ThreadFini, 0);

        // Register function to be called when the application exits
        PIN_AddFiniFunction(Fini, 0);
    }

    //cerr << "===============================================" << endl;
    //cerr << "This application is instrumented by MyPinTool" << endl;
    //if (!KnobOutputFile.Value().empty())
    //{
    //    cerr << "See file " << KnobOutputFile.Value() << " for analysis results" << endl;
    //}
    //cerr << "===============================================" << endl;

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
