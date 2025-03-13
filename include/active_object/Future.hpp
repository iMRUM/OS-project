//
// Created by imry on 3/12/25.
//

#ifndef FUTURE_HPP
#define FUTURE_HPP

template<class T>
class Future {
    // This class implements a ‘single write, multiple
    // read’ pattern that can be used to return results
    // from asynchronous method invocations.
public:
    // Constructor.
    Future(void);

    // Copy constructor that binds <this> and <r> to
    // the same <Future> representation
    Future(const Future<T> &r);

    // Destructor.
    ~Future(void);

    // Assignment operator that binds <this> and
    // <r> to the same <Future>.
    void operator =(const Future<T> &r);

    // Cancel a <Future>. Put the future into its
    // initial state. Returns 0 on success and -1
    // on failure.
    int cancel(void);

    // Type conversion, which obtains the result
    // of the asynchronous method invocation.
    // Will block forever until the result is
    // obtained.
    operator T();

    // Check if the result is available.
    int ready(void);

private:
    Future_Rep<T> *future_rep_;
    // Future representation implemented using
    // the Counted Pointer idiom.
};
#endif //FUTURE_HPP
