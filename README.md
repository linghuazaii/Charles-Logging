# Charles' Simple Logging System
This project is to be aimed to be simple and just for learning purpose. And you are free to modify and redistribute it under the author's name. It's license is GPL, though I don't know much details of GPL license. View the restrictions here.[GPL](https://github.com/linghuazaii/Charles-Logging/blob/master/LICENSE)

### Design Purpose
I have found logging in my company's project so stupid that logging functions like `LOG_ERROR` or anything else will block business handling. When the logging grows bigger and bigger, then it becomes a monster which eats several milliconds in time cost of request handling. So stupid, Huh!   
Even if `write` with `APPEND` to the same file won't be interleaved in multi-threading situation for now? Won't it be in the future file system? So a wise design is using Producer/Consumer pattern. Multiple threads write logging messages to the logging queue and only one thread consumes messages from the queue. The consumer thread works in `IDLE` mode not to affect the global performance. Just think about it, perfect! **PERFECT!!!**  
Now, let's implement it!
