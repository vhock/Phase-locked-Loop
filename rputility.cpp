#include "rputility.h"

void RPUtility::connect(std::string ipAddress){
    boost::system::error_code ec;
    boost::asio::ip::address::from_string( ipAddress, ec );
    if ( ec )
        std::cerr << ec.message( ) << std::endl;
}
RPUtility::RPUtility()
{

}
