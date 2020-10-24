/*
 * Copyright 2002-2019 Intel Corporation.
 *
 * This software is provided to you as Sample Source Code as defined in the accompanying
 * End User License Agreement for the Intel(R) Software Development Products ("Agreement")
 * section 1.L.
 *
 * This software and the related documents are provided as is, with no express or implied
 * warranties, other than those that are expressly stated in the License.
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;

ofstream OutFile;

static int bbcount;

// This function is called before every block
VOID PIN_FAST_ANALYSIS_CALL docount() { bbcount++; }

// Pin calls this function every time a new basic block is encountered
VOID Trace(TRACE trace, VOID *v)
{
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl);
       bbl = BBL_Next(bbl))
  {
    BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),
                   IARG_FAST_ANALYSIS_CALL, IARG_END);
  }
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
                            "o", "warmup1_bbcount.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
  // Write to a file since cout and cerr maybe closed by the application
  OutFile.setf(ios::showbase);
  OutFile << "Count " << bbcount << endl;
  OutFile.close();
}


INT32 Usage()
{
  cerr << "This tool counts the number of basic blocks" << endl;
  cerr << endl
       << KNOB_BASE::StringKnobSummary() << endl;
  return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
  // Initialize pin
  if (PIN_Init(argc, argv))
    return Usage();

  OutFile.open(KnobOutputFile.Value().c_str());

  // Register Instruction to be called to instrument instructions
  TRACE_AddInstrumentFunction(Trace, 0);

  // Register Fini to be called when the application exits
  PIN_AddFiniFunction(Fini, 0);

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}
