/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <limits>

#include "delay_queue.hh"
#include "timestamp.hh"

using namespace std;

const auto start = timestamp();

void DelayQueue::read_packet( const string & contents )
{
    const auto now = timestamp();
    /*if(now - start < 30000){
        packet_queue_.emplace( now + 50, contents );
    }else{
        packet_queue_.emplace( now + 100, contents );
    }*/

    int sec_elapsed = (now - start) / 10;//1000; //get the number of seconds elapsed after emulation starts 1000-> every sec, 1-> every ms
    int row_num = sec_elapsed % 4;//2000; //number of rows in the file; the delay values wrap around the input file

    FILE* file = fopen("/home/ubuntu/second_lat_mapping.txt", "r");
    char line[256];
    int i = 0;
    float delay = 0.0;
    while (fgets(line, sizeof(line), file)) {
        if(i == row_num )
        {
            delay = atof(line);
            packet_queue_.emplace( now + delay, contents );
            break;
        }
        i++;
    }
    fclose(file);

    //packet_queue_.emplace( now + delay_ms_, contents );
}

void DelayQueue::write_packets( FileDescriptor & fd )
{
    /*while ( (!packet_queue_.empty())
            && (packet_queue_.front().first <= timestamp()) ) {
        fd.write( packet_queue_.front().second );
        packet_queue_.pop();
    }*/

    int qSize = packet_queue_.size();
    while ( qSize > 0){
        qSize--;
        if(packet_queue_.front().first > timestamp()){
            packet_queue_.emplace( packet_queue_.front().first, packet_queue_.front().second );
        }else{
            fd.write( packet_queue_.front().second );
        }
        packet_queue_.pop();
    }
}

unsigned int DelayQueue::wait_time( void ) const
{
    if ( packet_queue_.empty() ) {
        return numeric_limits<uint16_t>::max();
    }

    const auto now = timestamp();

    if ( packet_queue_.front().first <= now ) {
        return 0;
    } else {
        return packet_queue_.front().first - now;
    }
}
