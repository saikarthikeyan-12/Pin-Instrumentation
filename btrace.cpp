#include <stdio.h>
#include "pin.H"
#include <map>
#include <iostream>
#include <iterator>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/syscall.h>
using namespace std;
static int flag = 0;
ofstream outFile;
static int sys = 0;

string evaluate_access_modes(long mode)
{
	string evaluated_access_modes = "";
	if (mode & R_OK)
	{
		evaluated_access_modes += "R_OK | ";
	}
	if (mode & W_OK)
	{
		evaluated_access_modes += "W_OK | ";
	}
	if (mode & X_OK)
	{
		evaluated_access_modes += "X_OK | ";
	}
	if (evaluated_access_modes.compare("") == 0)
	{
		evaluated_access_modes += "F_OK";
	}
	return evaluated_access_modes;
}

string evaluate_file_flags(long flags)
{
	string evaluated_file_flags = "O_RDONLY | ";
	if (flags & O_CLOEXEC)
	{
		evaluated_file_flags += "O_CLOEXEC | ";
	}
	if (flags & O_CREAT)
	{
		evaluated_file_flags += "O_CREAT | ";
	}
	if (flags & O_TRUNC)
	{
		evaluated_file_flags += "O_TRUNC | ";
	}
	if (flags & O_APPEND)
	{
		evaluated_file_flags += "O_APPEND | ";
	}
	if (flags & O_WRONLY)
	{
		evaluated_file_flags += "O_WRONLY | ";
	}
	if (flags & O_RDWR)
	{
		evaluated_file_flags += "O_RDWR | ";
	}
	return evaluated_file_flags;
}

string evaluate_mem_flags(long flags)
{
	string evaluated_mem_flags = "PROT_READ | ";
	if (flags & PROT_WRITE)
	{
		evaluated_mem_flags += "PROT_WRITE | ";
	}
	if (flags & PROT_EXEC)
	{
		evaluated_mem_flags += "PROT_EXEC | ";
	}
	return evaluated_mem_flags;
}

// This function is called before every instruction is executed and prints the system call name and its arguments.
VOID SystemCallBefore(VOID *ip, int num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4, ADDRINT arg5)
{
	sys = num;
	if (num == SYS_open)
	{
		char *argument0 = (char *)arg0;
		string evaluated_file_flags = evaluate_file_flags((long)arg1);
		outFile << "open"
				<< "(" << argument0 << "," << evaluated_file_flags << ")";
	}
	else if (num == SYS_read)
	{
		unsigned long argument0 = (unsigned long)arg0;
		unsigned long argument2 = (unsigned long)arg2;
		outFile << "read"
				<< "(" << argument0 << "," << arg1 << "," << argument2 << ")";
	}
	else if (num == SYS_write)
	{
		int filedescriptor = (int)arg0;
		char *argument1 = (char *)arg1;
		int argument2 = (int)arg2;
		outFile << "write"
				<< "(" << filedescriptor << "," << argument1 << "," << argument2 << ")";
	}
	else if (num == SYS_close)
	{
		outFile << "close"
				<< "(" << arg0 << ")";
	}
	else if (num == SYS_access)
	{
		char *argument0 = (char *)arg0;
		unsigned long argument1 = (unsigned long)arg1;
		string evaluated_access_modes = evaluate_access_modes(argument1);
		outFile << "access"
				<< "(" << argument0 << "," << evaluated_access_modes << ")";
	}
	else if (num == SYS_mprotect)
	{
		unsigned long argument0 = (unsigned long)arg0;
		size_t argument1 = (size_t)arg1;
		unsigned long argument2 = (unsigned long)arg2;
		string evaluated_mem_flags = evaluate_mem_flags(argument2);
		outFile << "mprotect"
				<< "(" << (void *)argument0 << "," << argument1 << "," << evaluated_mem_flags << ")";
	}

	else if (num == SYS_fstatfs)
	{
		struct statfs *argument1 = (struct statfs *)arg1;
		outFile << "fstatfs"
				<< "(" << arg0 << "," << argument1 << ")\n";
	}
	else if (num == SYS_fstat)
	{
		struct __old_kernel_stat *argument1 = (struct __old_kernel_stat *)arg1;
		outFile << "fstat"
				<< "(" << arg0 << "," << argument1 << ")\n";
	}
	else if (num == SYS_brk)
	{
		unsigned long argument0 = (unsigned long)arg0;
		if (argument0 == 0)
		{
			outFile << "brk"
					<< "("
					<< "NULL"
					<< ")";
		}
		else
		{
			outFile << "brk"
					<< "(" << argument0 << ")";
		}
	}
	else if (num == SYS_munmap)
	{
		unsigned long argument0 = (unsigned long)arg0;
		size_t argument1 = (size_t)arg1;
		outFile << "mmap"
				<< "(" << argument0 << "," << argument1 << ")";
	}

	else if (num == SYS_mmap2)
	{
		void *argument0 = (void *)arg0;
		size_t argument1 = (size_t)arg1;
		unsigned long argument2 = (unsigned long)arg2;
		string evaluated_mem_flags = evaluate_mem_flags(argument2);
		if (argument0 == 0x00000000)
		{
			outFile << "mmap2"
					<< "("
					<< "NULL"
					<< "," << argument1 << "," << evaluated_mem_flags << "," << arg3 << "," << (int)arg4 << "," << arg5 << ")";
		}
		else
		{
			outFile << "mmap2"
					<< "(" << argument0 << "," << argument1 << "," << evaluated_mem_flags << "," << arg3 << "," << (int)arg4 << "," << arg5 << ")";
		}
	}

	flag = 1;
}
//This is called at the start of every block when a system call is encountered.(Checked through the flag).
VOID SystemCallAfter(ADDRINT ret)
{
	if (flag == 1)
	{

		if (sys == SYS_open)
		{
			int returnval = (int)ret;
			outFile << "=" << returnval << "\n";
		}

		else if (sys == SYS_read)
		{
			int returnval = (int)ret;
			outFile << "=" << returnval << "\n";
		}
		else if (sys == SYS_write)
		{
			int returnval = (int)ret;
			outFile << "=" << returnval << "\n";
		}
		else if (sys == SYS_close)
		{
			int returnval = (int)ret;
			outFile << "=" << returnval << "\n";
		}
		else if (sys == SYS_access)
		{
			int returnval = (int)ret;
			if (returnval < 0)
			{
				outFile << "=" << -1 << "\t"
						<< "ENOENT (No such file or directory)"
						<< "\n";
			}
			else
			{
				outFile << "=" << returnval << "\n";
			}
		}

		else if (sys == SYS_mprotect)
		{
			int returnval = (int)ret;
			outFile << "=" << returnval << "\n";
		}
		else if (sys == SYS_fstatfs)
		{
			int returnval = (int)ret;
			outFile << "=" << returnval << "\n";
		}
		else if (sys == SYS_fstat)
		{
			int returnval = (int)ret;
			outFile << "=" << returnval << "\n";
		}

		else if (sys == SYS_brk)
		{
			int returnval = (int)ret;
			outFile << "=" << (void *)returnval << "\n";
		}

		else if (sys == SYS_munmap)
		{
			int returnval = (int)ret;
			outFile << "=" << returnval << "\n";
		}
		else if (sys == SYS_mmap2)
		{
			int returnval = (int)ret;
			outFile << "=" << (void *)returnval << "\n";
		}
	}
	flag = 0;
}

VOID Trace(TRACE trace, VOID *v)
{
	for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
	{
		TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(SystemCallAfter), IARG_REG_VALUE, REG_EAX, IARG_END);
		for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
		{

			if (INS_IsSyscall(ins))
			{

				INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SystemCallBefore),
							   IARG_INST_PTR, IARG_SYSCALL_NUMBER,
							   IARG_SYSARG_VALUE, 0, IARG_SYSARG_VALUE, 1,
							   IARG_SYSARG_VALUE, 2, IARG_SYSARG_VALUE, 3,
							   IARG_SYSARG_VALUE, 4, IARG_SYSARG_VALUE, 5, IARG_END);
			}
		}
	}
}
// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
	// Write to a file since cout and cerr maybe closed by the application
	outFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
INT32 Usage()
{
	PIN_ERROR("This Pintool prints the IPs of every instruction executed\n" + KNOB_BASE::StringKnobSummary() + "\n");
	return -1;
}
/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
int main(int argc, char *argv[])
{

	outFile.open("btrace.out");

	// Initialize pin
	if (PIN_Init(argc, argv))
		return Usage();
	// Register Instruction to be called to instrument instructions
	TRACE_AddInstrumentFunction(Trace, 0);
	// Register Fini to be called when the application exits
	PIN_AddFiniFunction(Fini, 0);

	// Start the program, never returns
	PIN_StartProgram();

	return 0;
}