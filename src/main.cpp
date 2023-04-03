#include "webhook_manager.h"

int main(int argc, char **argv)
{
    while(true) {
        ohtoai::WebhookManager a(argc, argv);
        try
        {
            switch(a.exec())
            {
                case ohtoai::Finish:
                    return 0;
                case ohtoai::Terminate:
                    return -1;
                case ohtoai::Reload:
                default:
                    break;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return -1;
        }
    }
}
