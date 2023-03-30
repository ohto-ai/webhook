#include "webhook_manager.h"

int main(int argc, char **argv)
{
    ohtoai::WebhookManager a(argc, argv);
    return a.exec();
}
