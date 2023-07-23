public class OscAddress
{
    static int PORT;
}

fun int parse_port(string str)
{
    Std.atoi(str) => int port;
    if (port <= 0)
    {
        9000 => int defaultPort;
        <<< "Osc Server is defaulting to port", defaultPort >>>;
        return defaultPort;
    }
    return port;
}

Std.getenv("OSC_PORT") => parse_port => OscAddress.PORT;