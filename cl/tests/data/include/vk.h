#ifndef H_GUARD_VK_H
#define H_GUARD_VK_H

enum vk_var_state_e {
    VK_DEAD = 0,
    VK_LIVE = 1
};

void VK_ASSERT(enum vk_var_state_e, ...);

#endif /* H_GUARD_VK_H */
