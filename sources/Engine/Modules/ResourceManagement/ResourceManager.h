#pragma once

#include <unordered_map>
#include <type_traits>
#include <typeindex>
#include <functional>

#include "Resource.h"
#include "ResourceInstance.h"

#include "Utility/xml.h"

class ResourceManager : public std::enable_shared_from_this<ResourceManager>
{
public:
    ResourceManager();
    ~ResourceManager();

    template <class T>
    void declareResourceType();

    template <class T>
    void declareResourceType(const std::string& mapAlias);

    template <class T>
    void declareResouceMapAlias(const std::string& alias);

    template <class T>
    void declareResource(const std::string& resourceId, const ResourceDeclaration& source);

    template<class T>
    T* getResourceFromInstance(const std::string& resourceId);

    const ResourceDeclaration& getResourceDeclaration(const std::string& resourceId);
    std::shared_ptr<ResourceInstance> getResourceInstance(const std::string& resourceId);

    void addResourcesMap(const std::string& path);

private:
    std::unordered_map<std::string, ResourceDeclaration> m_resourcesSources;
    std::unordered_map<std::string, std::shared_ptr<ResourceInstance>> m_resourcesInstances;

    std::unordered_map<std::type_index, std::function<Resource* ()>> m_resourcesFactories;
    std::unordered_map<std::string, std::type_index> m_resourcesTypesIds;

    std::unordered_map<std::string,
    std::function<void(const std::string&, const ResourceSource&, const pugi::xml_node&)>> m_resourcesDeclarers;
};

template<class T>
T* ResourceManager::getResourceFromInstance(const std::string& resourceId)
{
    return getResourceInstance(resourceId)->getResource<T>();
}

template<class T>
void ResourceManager::declareResourceType(const std::string& mapAlias)
{
    static_assert (std::is_base_of_v<Resource, T>);

    declareResourceType<T>();
    declareResouceMapAlias<T>(mapAlias);
}

template<class T>
void ResourceManager::declareResouceMapAlias(const std::string& alias)
{
    static_assert (std::is_base_of_v<Resource, T>);

    std::function declarer = [=] (const std::string& resourceId, const ResourceSource& source,
            const pugi::xml_node& declarationNode)
    {
        std::any parameters = T::buildDeclarationParameters(declarationNode);
        declareResource<T>(resourceId, ResourceDeclaration{ source, parameters });
    };

    m_resourcesDeclarers.insert({ alias, declarer });
}

template<class T>
void ResourceManager::declareResourceType()
{
    static_assert (std::is_base_of_v<Resource, T>);

    m_resourcesFactories.insert({ std::type_index(typeid(T)), [] () {
        return new T();
    }});
}

template<class T>
void ResourceManager::declareResource(const std::string &resourceId, const ResourceDeclaration &source)
{
    static_assert (std::is_base_of_v<Resource, T>);

    m_resourcesSources.insert({ resourceId, source });
    m_resourcesTypesIds.insert({ resourceId, std::type_index(typeid (T)) });
}
