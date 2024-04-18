class Environment:
    def Clean(self, targets, files):
        pass

    def GetOption(self, name: str) -> bool:
        if name == 'clean':
            return False
        return False
