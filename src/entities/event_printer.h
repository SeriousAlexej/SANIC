#ifndef EVENT_PRINTER_H_INCLUDED
#define EVENT_PRINTER_H_INCLUDED
#include <entity.h>

class EventPrinter : public Entity
{
public:
    FROM_INCUBATOR(EventPrinter);

    EventPrinter();
    virtual ~EventPrinter();
    virtual void initialize();

private:
    DECLARE_STATE(main);

};

#endif // EVENT_PRINTER_H_INCLUDED
