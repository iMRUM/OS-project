#ifndef METHODREQUEST_HPP
#define METHODREQUEST_HPP

/**
 * The Method Request abstract class defines an interface for
 * executing methods of an Active Object. It contains guard methods
 * to determine when synchronization constraints are met.
 *
 * For every Active Object method that requires synchronized access,
 * this class is subclassed to create concrete Method Request classes.
 */
class MethodRequest {
public:
    MethodRequest() = default;
    virtual ~MethodRequest() = default;

    // Evaluate synchronization constraint
    // Returns true if the request can be executed
    virtual bool guard() const = 0;

    // Execute the method
    virtual void call() = 0;
};

#endif //METHODREQUEST_HPP