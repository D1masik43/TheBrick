    #pragma once
    #include <string>

    class StaticApp
    {
    public:

        StaticApp(std::string name);

        virtual void Loop() = 0;
        virtual void Update(int button) = 0;
        virtual void Setup() = 0;
        virtual void Draw() = 0;
        std::string GetName();

        void SetName(std::string name);

        virtual const unsigned char *getIcon();

    private:
        std::string mName;
    };
