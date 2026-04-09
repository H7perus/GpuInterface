#include "TaskGraph.h"

void KE::VK::TaskGraph::Compile()
{
    // RESOLVE DEPENDENCIES
    for (auto& node : tasks_)
    {
        for (auto& entry : node->reads_)
        {
            if (namedResources_.contains(entry.first))
            {
                entry.second = namedResources_[entry.first];
            }
        }
        for (auto& entry : node->writes_)
        {
            if (namedResources_.contains(entry.first))
            {
                entry.second = namedResources_[entry.first];
            }
        }
    }

    // INSERT BARRIERS
    std::vector<std::string> resourcesWritten;
    for (int i = 0; i < tasks_.size(); i++)
    {
        std::unique_ptr<ITaskNode>& node = tasks_[i];
    }
}
void KE::VK::TaskGraph::Execute(vk::CommandBuffer cmd)
{
    for (auto& task : tasks_)
    {
        task->Execute(cmd);
    }
}
void KE::VK::TaskGraph::AddNamedResource(std::string name, ResourceHandle resource)
{
    if (!namedResources_.contains(name))
    {
        namedResources_.insert({name, resource});
    }
    else
    {
        throw std::logic_error(std::format("Resource with name \"{}\" already exists!", name));
    }
};
