#pragma once

#include "TaskNode.h"

#include <memory>
#include <vector>


#include "GpuInterfaceDLL.h"

namespace KE::VK
{

class GPUI_DLL_API TaskGraph
{
  public:
    TaskGraph()  = default;
    ~TaskGraph() = default;

    TaskGraph(const TaskGraph&)            = delete;
    TaskGraph& operator=(const TaskGraph&) = delete;
    TaskGraph(TaskGraph&&)                 = default;
    TaskGraph& operator=(TaskGraph&&)      = default;

    template <std::derived_from<ITaskNode> T> 
    void AddTask(auto&&... args)
    {
        tasks_.emplace_back(std::make_unique<T>(std::forward<decltype(args)>(args)...));
    }

    void Compile();

    void Execute(vk::CommandBuffer cmd);

    void AddNamedResource(std::string name, ResourceHandle resource);

  private:
    std::vector<std::unique_ptr<ITaskNode>> tasks_;

    std::map<std::string, ResourceHandle> namedResources_;
};

} // namespace KE::VK