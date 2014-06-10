/*createRequest.h*/
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * pvDataCPP is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */

#ifndef CREATEREQUEST_H
#define CREATEREQUEST_H
#include <string>
#include <sstream>

#ifdef epicsExportSharedSymbols
#   define pvDataEpicsExportSharedSymbols
#   undef epicsExportSharedSymbols
#endif


#include <pv/pvData.h>
#include <pv/lock.h>

#ifdef pvDataEpicsExportSharedSymbols
#   define epicsExportSharedSymbols
#       undef pvDataEpicsExportSharedSymbols
#endif



namespace epics { namespace pvData {

/**
 * Class to create pvRequest structures to pass to pvAccess Channel methods.
 */
class epicsShareClass CreateRequest {
    public:
    POINTER_DEFINITIONS(CreateRequest);
    /**
     * Create s new instance of CreateRequest
     * @returns A shared pointer to the new instance.
     */
    static CreateRequest::shared_pointer create();
    virtual ~CreateRequest() {};
    /**
    * Create a request structure for the create calls in Channel.
    * See the package overview documentation for details.
    * @param request The field request. See the package overview documentation for details.
    * @param requester The requester;
    * @return The request PVStructure if a valid request was given.
    * If a NULL PVStructure is returned then getMessage will return
    * the reason.
    */
    virtual PVStructure::shared_pointer createRequest(String const & request) = 0;
    /**
     * Get the error message of createRequest returns NULL
     * return the error message
     */
    String getMessage() {return message;}
protected:
    CreateRequest() {}
    String message;
};


}}

#endif /* CREATEREQUEST_H */
