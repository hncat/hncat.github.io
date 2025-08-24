import { sidebar } from "vuepress-theme-hope";

export default sidebar({
  "/": [
    "",
    {
      text: "c/c++",
      icon: "code",
      prefix: "code/",
      link: "code/",
      children: "structure",
    },
    {
      text: "计算机基础",
      icon: "book",
      prefix: "cs/",
      link: "cs/",
      collapsible: true,
      children: [{
        text: "链接、装载和库",
        icon: "book",
        prefix: "linker/",
        link: "linker/",
        collapsible: true,
        children: "structure"
      }],
    },
    {
      text: "数据库",
      icon: "database",
      prefix: "database/",
      link: "database/",
      children: [{
        text: "mysql基础",
        icon: "database",
        prefix: "basic/",
        link: "basic/",
        collapsible: true,
        children: "structure"
      }],
    },
    "intro",
  ],
});
