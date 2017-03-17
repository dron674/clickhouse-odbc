#include "statement.h"

Statement::Statement(Connection & conn_)
    : connection(conn_)
{
}

void Statement::sendRequest()
{
    if (connection.user.find(':') != std::string::npos)
        throw std::runtime_error("Username couldn't contain ':' (colon) symbol.");

    std::ostringstream user_password_base64;
    Poco::Base64Encoder base64_encoder(user_password_base64);
    base64_encoder << connection.user << ":" << connection.password;
    base64_encoder.close();

    Poco::Net::HTTPRequest request;

    request.setMethod(Poco::Net::HTTPRequest::HTTP_POST);
    request.setVersion(Poco::Net::HTTPRequest::HTTP_1_1);
    request.setKeepAlive(true);
    request.setChunkedTransferEncoding(true);
    request.setCredentials("Basic", user_password_base64.str());
    request.setURI("/?database=" + connection.database + "&default_format=ODBCDriver"); /// TODO Ability to transfer settings. TODO escaping

//      if (in && in->peek() != EOF)
    //XXX     connection.session.reset();

    connection.session.sendRequest(request) << query;

    LOG("Receiving !");
    response = std::make_unique<Poco::Net::HTTPResponse>();
    in = &connection.session.receiveResponse(*response);
    LOG("Receiving !!");

    Poco::Net::HTTPResponse::HTTPStatus status = response->getStatus();
    LOG("Receiving !!!");

    if (status != Poco::Net::HTTPResponse::HTTP_OK)
    {
        LOG("Receiving !!!!");

        std::stringstream error_message;
        error_message
            << "HTTP status code: " << status << std::endl
            << "Received error:" << std::endl
            << in->rdbuf() << std::endl;

        throw std::runtime_error(error_message.str());
    }

    LOG("Receiving ! !");
    result.init(*this);
}

bool Statement::fetchRow()
{
    current_row = result.fetch();
    return current_row;
}

void Statement::reset()
{
    in = nullptr;
    response.reset();
    //XXX connection.session.reset();
    diagnostic_record.reset();
    result = ResultSet();
}
