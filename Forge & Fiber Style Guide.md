# **Forge & Fiber: Systems Engineering Style Guide**

## **1\. The "Safety-First" Mandate**

At Forge & Fiber, we build for reliability. Our code powers hardware where failure isn't just a bug—it’s a liability.

* **Predictability \> Conciseness:** No "magic" code or hidden side effects.  
* **Memory Discipline:** Minimize dynamic allocations; prefer stack over heap.  
* **Fail Fast:** Validate inputs at the boundary. If something is wrong, crash or return an error immediately rather than propagating "garbage" data.

## ---

**2\. Structural Standards**

### **File Organization**

Files should be kept small and focused. Each file must follow this specific order:

1. **License Header** (SPDX format)  
2. **Imports/Includes** (Grouped: Standard Lib, Third-party, Internal)  
3. **Constants & Macros**  
4. **Type Definitions / Interfaces**  
5. **Main Implementation**

### **Indentation and Spacing**

| Element | Rule |
| :---- | :---- |
| **Indentation** | **4 Spaces.** Higher visual contrast for nested logic. |
| **Brace Style** | **Allman Style** (Braces on new lines). |
| **Spacing** | Single space after keywords (if, while, switch). |

## ---

**3\. Naming Conventions: Hungarian-Light**

To prevent type-related errors in systems languages, we use a modified prefix system.

* **p\_** for Pointers (e.g., p\_buffer)  
* **g\_** for Global variables (use sparingly)  
* **s\_** for Static variables  
* **m\_** for Class members/fields  
* **is\_** or **has\_** for Booleans

## ---

**4\. Error Handling**

We do not use silent failures. All functions that can fail must communicate the reason.

### **The Result Pattern**

Prefer returning a Result object or an Error Code over throwing exceptions, which can be expensive and unpredictable in real-time systems.

C++

// Preferred: Explicit error checking  
status\_t result \= hardware\_init(p\_config);  
if (result \!= STATUS\_OK)   
{  
    log\_error("Hardware init failed: %d", result);  
    return result;  
}

## ---

**5\. Commenting & Metadata**

We use **Doxygen** for all documentation. Code without 100% doc coverage will not pass CI.

* **In-line comments:** Must use // and start with a capital letter.  
* **TODOs:** Must include a username and a ticket reference: // TODO(jdoe): FF-104 \- Optimize this loop.

## ---

**6\. Memory & Performance**

* **No Raw Pointers:** Use smart pointers or references unless interfacing with legacy C APIs.  
* **Immutable by Default:** Use const (or final/readonly) for every variable unless it absolutely must change.  
* **Loop Efficiency:** Prefer pre-increment (++i) over post-increment (i++) to avoid unnecessary copies in complex iterators.

## ---

**7\. Review Checklist**

Before submitting a Pull Request (PR), ensure:

* \[ \] No compiler warnings (Warnings \= Errors).  
* \[ \] Unit test coverage is \> 90%.  
* \[ \] Complexity score (Cyclomatic) is below 10 for any single function.  
* \[ \] No "Magic Numbers"—all constants are named.

---

**Would you like me to create a "Reviewer's Checklist" template that your team can use during PRs to enforce these rules?**