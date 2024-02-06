#pragma once
#include<type_traits>

class GameObject;
class GameObjectComponentBase;

template<typename T>
concept GameObjectClass = std::is_base_of<GameObject, T>::value;

template<typename T>
concept GameObjectComponentClass = std::is_base_of<GameObjectComponentBase, T>::value && !std::is_same<GameObjectComponentBase, T>::value;

template<typename ComponentClass>
struct ComponentTemplate {};
