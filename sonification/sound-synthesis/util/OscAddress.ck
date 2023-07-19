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

public class OscAddress
{
    static int __port;
    fun static int PORT()
    {
        if (__port == 0)
        {
            Std.getenv("OSC_PORT") => parse_port => __port;
        }
        return __port;
    }
}
